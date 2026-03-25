#!/usr/bin/env python3
"""
concrete_verify.py — Verify SAILOR findings against upstream HEAD.

Clones an upstream project, builds it with ASan, compiles SAILOR's
replay reproducers against it, and classifies whether each finding
reproduces concretely (CONFIRMED / NOT_REPRODUCIBLE / PARTIAL / etc.).

Usage:
    python3 sailor_engine/concrete_verify.py \
        --results-dir se_runs/sailor_engine/libxml2_62911_vul \
        --upstream-url https://gitlab.gnome.org/GNOME/libxml2.git \
        --configure-args "--with-python=no --without-http --without-ftp"
"""

import argparse
import csv
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import textwrap
from datetime import datetime
from pathlib import Path

from asan_utils import parse_asan_frames, is_harness_crash as _is_harness_crash_raw


# ── Verdicts ─────────────────────────────────────────────────────────

CONFIRMED = "CONCRETE_CONFIRMED"
NOT_REPRODUCIBLE = "NOT_REPRODUCIBLE"
PARTIAL = "PARTIAL"
REPRODUCER_BUILD_FAIL = "REPRODUCER_BUILD_FAIL"
UPSTREAM_BUILD_FAIL = "UPSTREAM_BUILD_FAIL"
INCONCLUSIVE = "INCONCLUSIVE"
HARNESS_ARTIFACT = "HARNESS_ARTIFACT"

# OSS-Fuzz validation verdicts
FUZZ_REPRODUCED = "FUZZ_REPRODUCED"
FUZZ_PARTIAL = "FUZZ_PARTIAL"
FUZZ_NO_CRASH = "FUZZ_NO_CRASH"
FUZZ_BUILD_FAIL = "FUZZ_BUILD_FAIL"
NOT_FUZZABLE = "NOT_FUZZABLE"


# ── Helpers ──────────────────────────────────────────────────────────

def run_cmd(cmd, timeout=120, cwd=None, env=None):
    """Run a command, return (returncode, stdout, stderr, elapsed)."""
    start = datetime.now()
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
            cwd=cwd,
            env=env,
        )
        elapsed = (datetime.now() - start).total_seconds()
        return proc.returncode, proc.stdout.decode(errors="replace"), proc.stderr.decode(errors="replace"), elapsed
    except subprocess.TimeoutExpired:
        elapsed = (datetime.now() - start).total_seconds()
        return -1, "", f"TIMEOUT after {elapsed:.0f}s", elapsed
    except Exception as e:
        elapsed = (datetime.now() - start).total_seconds()
        return -1, "", str(e), elapsed


def parse_asan_output(combined):
    """Parse ASan error type and crash location from combined stdout+stderr."""
    info = {"asan_triggered": "AddressSanitizer" in combined}
    if info["asan_triggered"]:
        m_type = re.search(r'ERROR: AddressSanitizer:\s*(\S+)', combined)
        m_loc = re.search(r'#0\s+\S+\s+in\s+(\S+)\s+(\S+):(\d+)', combined)
        if m_type:
            info["error_type"] = m_type.group(1)
        if m_loc:
            info["function"] = m_loc.group(1)
            info["file"] = m_loc.group(2)
            info["line"] = int(m_loc.group(3))
        # Collect all stack frames for harness-artifact detection
        info["frames"] = parse_asan_frames(combined)
    return info


def _is_harness_crash(upstream_asan):
    """Thin wrapper: extract frames from upstream_asan dict and delegate."""
    return _is_harness_crash_raw(upstream_asan.get("frames", []))


# ── Stage 1: Scan Findings ──────────────────────────────────────────

def scan_findings(results_dir, spec_filter=None):
    """Read summary.tsv and collect TP/LIKELY_TP findings with their replay data."""
    summary_path = results_dir / "summary.tsv"
    if not summary_path.exists():
        print(f"[ERROR] summary.tsv not found at {summary_path}")
        sys.exit(1)

    with open(summary_path, "r") as f:
        reader = csv.DictReader(f, delimiter="\t")
        all_rows = list(reader)

    # Filter: SE_DETECTED / VALIDATED_TP status (covers both TP and LIKELY_TP verdicts)
    tp_statuses = {"SE_DETECTED", "VALIDATED_TP"}
    tp_rows = [r for r in all_rows if r.get("FinalStatus") in tp_statuses]
    if not tp_rows:
        print("[i] No SE_DETECTED/VALIDATED_TP specs found in summary.tsv")
        return []

    # Optionally filter to specific specs
    if spec_filter:
        tp_rows = [r for r in tp_rows if r["Spec"] in spec_filter]
        if not tp_rows:
            print(f"[WARN] None of the requested specs are SE_DETECTED")
            return []

    findings = []
    for row in tp_rows:
        spec = row["Spec"]
        spec_dir = results_dir / spec

        # Load bug_report.json for crash details
        bug_report_path = spec_dir / "bug_report.json"
        if not bug_report_path.exists():
            print(f"  [WARN] {spec}: no bug_report.json, skipping")
            continue

        with open(bug_report_path, "r") as f:
            bug_report = json.load(f)

        # Check for asan_replay directory
        replay_dir = spec_dir / "asan_replay"
        if not replay_dir.exists():
            print(f"  [WARN] {spec}: no asan_replay/ directory, skipping")
            continue

        replay_driver = replay_dir / "replay_driver.c"
        if not replay_driver.exists():
            print(f"  [WARN] {spec}: no replay_driver.c, skipping")
            continue

        # Collect all .c files in asan_replay (excluding the compiled binary)
        replay_c_files = sorted(replay_dir.glob("*.c"))
        replay_h_files = sorted(replay_dir.glob("*.h"))

        # Collect companion .c/.h files from harness/ (excluding driver.c)
        harness_dir = spec_dir / "harness"
        harness_companion_c = []
        harness_companion_h = []
        if harness_dir.is_dir():
            harness_companion_c = sorted(
                f for f in harness_dir.glob("*.c") if f.name != "driver.c"
            )
            harness_companion_h = sorted(
                f for f in harness_dir.glob("*.h") if f.name != "harness_types.h"
            )

        # Also check harness dir for harness_types.h
        harness_types_h = spec_dir / "harness" / "harness_types.h"

        # Extract crash info from bug_report
        asan_result = bug_report.get("asan_result", {})
        finding = {
            "spec": spec,
            "verdict": row.get("Verdict", ""),
            "bug_type": row.get("BugType", ""),
            "bug_file": row.get("BugFile", ""),
            "bug_line": row.get("BugLine", ""),
            "bug_func": row.get("BugFunc", ""),
            "cwe": row.get("CWE", "unknown"),
            "asan_error_type": asan_result.get("error_type", ""),
            "asan_function": asan_result.get("crash_function", ""),
            "asan_file": asan_result.get("original_file", ""),
            "asan_line": asan_result.get("original_line", ""),
            "original_context": bug_report.get("vulnerability", {}).get("original_context", ""),
            "concrete_inputs": bug_report.get("concrete_inputs", []),
            "replay_dir": replay_dir,
            "replay_c_files": replay_c_files,
            "replay_h_files": replay_h_files,
            "harness_companion_c": harness_companion_c,
            "harness_companion_h": harness_companion_h,
            "harness_types_h": harness_types_h if harness_types_h.exists() else None,
            "asan_output": asan_result.get("output", ""),
        }
        findings.append(finding)
        print(f"  [+] {spec}: {finding['asan_error_type']} in {finding['bug_file']}:{finding['bug_line']}")

    return findings


# ── Stage 2: Clone & Build Upstream ─────────────────────────────────

def clone_upstream(upstream_url, upstream_branch, upstream_src_dir, upstream_commit=None):
    """Clone or update the upstream repo.

    If *upstream_commit* is given, the worktree is checked out at that exact
    commit so that verification runs against the analysed code, not HEAD.
    """
    if upstream_src_dir.exists() and (upstream_src_dir / ".git").exists():
        print(f"  [i] Upstream already cloned at {upstream_src_dir}, fetching...")
        run_cmd(
            ["git", "fetch", "origin", upstream_branch],
            timeout=120, cwd=str(upstream_src_dir))
    else:
        upstream_src_dir.parent.mkdir(parents=True, exist_ok=True)
        if upstream_commit:
            # Full clone so we can checkout the exact commit
            print(f"  [i] Cloning {upstream_url} (branch: {upstream_branch}, full history)...")
            rc, _, stderr, _ = run_cmd(
                ["git", "clone", "--branch", upstream_branch, upstream_url, str(upstream_src_dir)],
                timeout=600)
        else:
            print(f"  [i] Cloning {upstream_url} (branch: {upstream_branch})...")
            rc, _, stderr, _ = run_cmd(
                ["git", "clone", "--depth=1", "--branch", upstream_branch, upstream_url, str(upstream_src_dir)],
                timeout=300)
        if rc != 0:
            print(f"  [ERROR] Clone failed: {stderr[:500]}")
            return None

    # Checkout the exact commit if requested
    if upstream_commit:
        print(f"  [i] Checking out exact commit {upstream_commit[:12]}...")
        rc, _, stderr, _ = run_cmd(
            ["git", "checkout", upstream_commit],
            timeout=30, cwd=str(upstream_src_dir))
        if rc != 0:
            print(f"  [WARN] Could not checkout {upstream_commit[:12]}: {stderr[:300]}")
            print(f"  [WARN] Falling back to branch HEAD")
    else:
        run_cmd(["git", "checkout", upstream_branch], timeout=30, cwd=str(upstream_src_dir))
        run_cmd(["git", "reset", "--hard", f"origin/{upstream_branch}"], timeout=30, cwd=str(upstream_src_dir))

    # Record HEAD info
    rc, stdout, _, _ = run_cmd(
        ["git", "log", "-1", "--format=%H %aI %s"],
        timeout=10, cwd=str(upstream_src_dir))
    if rc == 0:
        parts = stdout.strip().split(" ", 2)
        return {
            "commit_hash": parts[0] if len(parts) > 0 else "unknown",
            "commit_date": parts[1] if len(parts) > 1 else "unknown",
            "commit_subject": parts[2] if len(parts) > 2 else "",
        }
    return {"commit_hash": "unknown", "commit_date": "unknown", "commit_subject": ""}


def detect_build_system(src_dir):
    """Auto-detect: cmake or autotools."""
    if (src_dir / "CMakeLists.txt").exists():
        return "cmake"
    if (src_dir / "configure").exists() or (src_dir / "configure.ac").exists():
        return "autotools"
    return None


def build_upstream_asan(upstream_src_dir, asan_build_dir, build_system, configure_args, compiler):
    """Build upstream with ASan instrumentation. Returns path to .a library or None."""
    asan_cflags = "-fsanitize=address -fno-omit-frame-pointer -g -O0"
    asan_ldflags = "-fsanitize=address"

    cc = compiler
    cxx = "g++" if compiler == "gcc" else "clang++"

    env = dict(os.environ)
    env["CC"] = cc
    env["CXX"] = cxx
    env["CFLAGS"] = asan_cflags
    env["CXXFLAGS"] = asan_cflags
    env["LDFLAGS"] = asan_ldflags
    # Remove wllvm/gllvm env vars that could interfere
    for k in ["LLVM_COMPILER", "LLVM_CC_NAME", "LLVM_CXX_NAME"]:
        env.pop(k, None)

    built_ok = False

    if build_system == "cmake":
        if asan_build_dir.exists():
            shutil.rmtree(str(asan_build_dir), ignore_errors=True)
        asan_build_dir.mkdir(parents=True, exist_ok=True)

        print(f"  [build] CMake configure (without ASan for probes)...")
        # Step 1: Configure WITHOUT ASan (ASan breaks cmake compiler probes)
        clean_env = dict(os.environ)
        clean_env["CC"] = cc
        clean_env["CXX"] = cxx
        for k in ["LLVM_COMPILER", "LLVM_CC_NAME", "LLVM_CXX_NAME"]:
            clean_env.pop(k, None)

        cmake_cmd = [
            "cmake", str(upstream_src_dir),
            f"-DCMAKE_C_COMPILER={cc}",
            f"-DCMAKE_CXX_COMPILER={cxx}",
            "-DBUILD_SHARED_LIBS=OFF",
        ]
        # Add user-provided configure args
        if configure_args:
            for arg in shlex.split(configure_args):
                cmake_cmd.append(arg)

        rc, _, stderr, _ = run_cmd(cmake_cmd, timeout=120, cwd=str(asan_build_dir), env=clean_env)
        if rc != 0:
            print(f"  [build] CMake configure failed: {stderr[:500]}")
            return None

        # Step 2: Make WITH ASan flags
        print(f"  [build] Building with ASan...")
        rc, _, stderr, _ = run_cmd(
            ["make", "-j4",
             f"CFLAGS={asan_cflags}",
             f"CXXFLAGS={asan_cflags}",
             f"LDFLAGS={asan_ldflags}"],
            timeout=600, cwd=str(asan_build_dir), env=env)
        if rc == 0:
            built_ok = True
        else:
            print(f"  [build] Make failed (partial build may still have library): {stderr[:300]}")
            built_ok = True  # library might still exist from partial build

    elif build_system == "autotools":
        asan_build_dir = upstream_src_dir  # autotools builds in-tree

        # Run autoreconf if configure doesn't exist
        if not (upstream_src_dir / "configure").exists():
            if (upstream_src_dir / "configure.ac").exists():
                print(f"  [build] Running autoreconf -fi...")
                rc, _, stderr, _ = run_cmd(
                    ["autoreconf", "-fi"], timeout=120, cwd=str(upstream_src_dir))
                if rc != 0:
                    print(f"  [build] autoreconf failed: {stderr[:500]}")
                    return None

        # Clean previous build
        run_cmd(["make", "distclean"], timeout=60, cwd=str(upstream_src_dir))

        configure_cmd = [
            str(upstream_src_dir / "configure"),
            "--disable-shared", "--enable-static",
        ]
        if configure_args:
            configure_cmd.extend(shlex.split(configure_args))

        print(f"  [build] Running configure...")
        rc, _, stderr, _ = run_cmd(configure_cmd, timeout=180, cwd=str(upstream_src_dir), env=env)
        if rc != 0:
            print(f"  [build] Configure failed: {stderr[:500]}")
            return None

        print(f"  [build] Building with ASan...")
        rc, _, stderr, _ = run_cmd(
            ["make", "-j4", "MAKEINFO=true"], timeout=600, cwd=str(upstream_src_dir), env=env)
        if rc == 0:
            built_ok = True
        else:
            print(f"  [build] Make failed (checking for partial library): {stderr[:300]}")
            built_ok = True
    else:
        print(f"  [build] Unknown build system")
        return None

    if not built_ok:
        return None

    # Search for resulting static libraries — collect ALL .a files
    search_dirs = [asan_build_dir]
    if asan_build_dir != upstream_src_dir:
        search_dirs.append(upstream_src_dir)

    all_libs = []
    for search_dir in search_dirs:
        found = list(search_dir.rglob("*.a"))
        found = [f for f in found if "CMakeFiles" not in str(f)
                 and "testsuite" not in str(f)
                 and "/noasan/" not in str(f)]
        all_libs.extend(found)

    # Deduplicate: if both .libs/libfoo.a and libfoo.a exist, prefer .libs/
    seen_names = {}
    for lib in all_libs:
        name = lib.name
        if name not in seen_names or ".libs" in str(lib):
            seen_names[name] = lib
    all_libs = sorted(seen_names.values(), key=lambda p: p.name)

    if all_libs:
        for lib in all_libs:
            print(f"  [build] Found ASan library: {lib}")
        return all_libs

    print(f"  [build] No .a files produced by build")
    return None


# ── Stage 3: Strip SAILOR Markers ───────────────────────────────────

SAILOR_MARKER_PATTERNS = [
    # Whole-line removals (lines containing these markers)
    (re.compile(r'^.*SAILOR_PROBE_SINK.*$', re.MULTILINE), ''),
    (re.compile(r'^.*SAILOR_SINK_REACHED.*$', re.MULTILINE), ''),
    (re.compile(r'^.*SAILOR_TRAP.*$', re.MULTILINE), ''),
    (re.compile(r'^.*SAILOR_SUSPECT_HIT.*$', re.MULTILINE), ''),
    (re.compile(r'^.*SAILOR_INST_BEGIN.*$', re.MULTILINE), ''),
    (re.compile(r'^.*SAILOR_INST_END.*$', re.MULTILINE), ''),
    # klee remnants
    (re.compile(r'^\s*#include\s*[<"]klee/klee\.h[>"]\s*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_assert\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_assume\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_make_symbolic\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_prefer_cex\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_print_range\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_warning_once\(.*$', re.MULTILINE), ''),
    (re.compile(r'^.*klee_warning\(.*$', re.MULTILINE), ''),
    # Comment markers from replay generation
    (re.compile(r'/\*\s*assert removed\s*\*/'), ''),
    (re.compile(r'/\*\s*probe removed\s*\*/'), ''),
    (re.compile(r'//\s*klee removed.*$', re.MULTILINE), ''),
    # Keep harness_types.h include — will be replaced with upstream headers when available
    # __sailor_entry_ prefix → reproducer_entry_
    (re.compile(r'__sailor_entry_'), 'reproducer_entry_'),
]


def strip_sailor_markers(source_text):
    """Remove SAILOR-specific markers and clean up the source."""
    text = source_text
    for pattern, replacement in SAILOR_MARKER_PATTERNS:
        text = pattern.sub(replacement, text)

    # Collapse multiple blank lines to at most two
    text = re.sub(r'\n{3,}', '\n\n', text)
    return text


def _generate_upstream_harness_types(upstream_src_dir, asan_build_dir):
    """Generate a harness_types.h that includes real upstream headers.

    Scans the upstream include directory for available headers and includes
    the most common ones.  Returns the upstream-only header content, or
    None if no project headers were found."""
    includes = []
    includes.append("#pragma once")
    includes.append("/* Upstream-aware harness_types.h — generated by concrete_verify.py */")

    # Discover available upstream headers
    header_dirs = []
    if upstream_src_dir:
        for d in ["include", ".", upstream_src_dir.name]:
            candidate = upstream_src_dir / d
            if candidate.is_dir():
                header_dirs.append(candidate)
    if asan_build_dir and asan_build_dir != upstream_src_dir:
        for d in ["include", ".", "config"]:
            candidate = asan_build_dir / d
            if candidate.is_dir():
                header_dirs.append(candidate)

    # Look for common project meta-headers (libxml/tree.h style)
    found_headers = set()
    for hdir in header_dirs:
        for hf in hdir.rglob("*.h"):
            rel = hf.relative_to(hdir)
            found_headers.add(str(rel))

    # Include strategy: include umbrella / commonly-needed headers
    # libxml2-specific but also works generically
    priority_headers = [
        "libxml/parser.h",
        "libxml/tree.h",
        "libxml/xmlschemas.h",
        "libxml/xmlschemastypes.h",
        "libxml/relaxng.h",
        "libxml/valid.h",
        "libxml/xpath.h",
        "libxml/xpointer.h",
        "libxml/catalog.h",
        "libxml/xmlreader.h",
        "libxml/xmlwriter.h",
        "libxml/list.h",
        "libxml/hash.h",
        "libxml/dict.h",
        "libxml/entities.h",
        "libxml/uri.h",
        "libxml/c14n.h",
        "libxml/xinclude.h",
        "libxml/HTMLparser.h",
        "libxml/HTMLtree.h",
        "libxml/debugXML.h",
        "libxml/xmlstring.h",
        "libxml/xmlmemory.h",
        "libxml/threads.h",
        "libxml/xmlIO.h",
        "libxml/xmlsave.h",
        "libxml/xmlregexp.h",
        "libxml/pattern.h",
        "libxml/xmlerror.h",
        "libxml/parserInternals.h",
        "libxml/globals.h",
    ]
    for hdr in priority_headers:
        if hdr in found_headers:
            includes.append(f"#include <{hdr}>")

    # Also include libxml.h if available (internal defines like IN_LIBXML)
    if "libxml.h" in found_headers:
        includes.append('#include "libxml.h"')

    # Fallback: if no project-specific headers found, keep stdlib basics
    if len(includes) <= 2:
        includes.append("#include <stddef.h>")
        includes.append("#include <stdint.h>")
        return None  # signal: no upstream headers found

    return "\n".join(includes) + "\n"


def _merge_harness_types(upstream_content: str, agent_content: str) -> str:
    """Merge upstream public headers with agent's custom type definitions.

    Strategy: put upstream #includes first (real types take priority),
    then append the agent's struct/typedef/enum/define definitions wrapped
    so they only take effect for types NOT already provided by the upstream
    headers.  Standard-library #includes and #pragma once from the agent
    header are dropped (upstream already has them or they'd conflict)."""

    merged = []
    merged.append("#pragma once")
    merged.append("/* Merged harness_types.h — upstream headers + agent definitions */")
    merged.append("/* Generated by concrete_verify.py */")
    merged.append("")

    # ── Part 1: upstream public headers ──
    for line in upstream_content.splitlines():
        stripped = line.strip()
        if stripped.startswith("#pragma once"):
            continue  # already added
        if stripped.startswith("/*") and ("verify_upstream" in stripped or "concrete_verify" in stripped):
            continue  # skip old comment
        merged.append(line)

    merged.append("")
    merged.append("/* ── Agent-defined types (for internal/private types not in public headers) ── */")
    merged.append("")

    # ── Part 2: agent definitions, filtered ──
    # Collect the names of structs/typedefs that upstream headers define so
    # we can skip agent re-definitions of the same names.  We can't know
    # for sure what upstream defines without compiling, but the wrapping in
    # #ifndef guards is safe: the compiler resolves it.
    #
    # We skip: #pragma once, stdlib #includes (already provided by upstream),
    # and the auto-generated comment.  We keep everything else — structs,
    # typedefs, enums, #defines, extern decls, inline functions.

    # Standard headers that upstream already pulls in
    _STDLIB_HEADERS = {
        "stdio.h", "stdlib.h", "string.h", "stddef.h", "stdint.h",
        "stdbool.h", "math.h", "limits.h", "assert.h", "errno.h",
        "ctype.h", "inttypes.h",
    }

    in_block_comment = False
    for line in agent_content.splitlines():
        stripped = line.strip()

        # Track block comments
        if "/*" in stripped and "*/" not in stripped:
            in_block_comment = True
        if in_block_comment:
            if "*/" in stripped:
                in_block_comment = False
            # Keep block comments from agent (they may document types)
            merged.append(line)
            continue

        # Skip #pragma once (already emitted)
        if stripped.startswith("#pragma once"):
            continue
        # Skip the AUTO-GENERATED comment header
        if stripped.startswith("/* AUTO-GENERATED"):
            continue

        # Skip standard-library includes (upstream headers pull these in)
        if stripped.startswith("#include"):
            m = re.search(r'#include\s*[<"]([^>"]+)[>"]', stripped)
            if m and m.group(1) in _STDLIB_HEADERS:
                continue

        merged.append(line)

    return "\n".join(merged) + "\n"


def prepare_reproducer(finding, finding_out_dir, upstream_src_dir=None, asan_build_dir=None):
    """Copy and clean asan_replay files into the finding output directory.

    Generates up to three versions of harness_types.h:
      1. harness_types.h          — merged (upstream headers + agent defs)
      2. harness_types_upstream.h — upstream-only (strongest verification)
      3. harness_types_agent.h    — agent-original (fallback)

    harness_types.h starts as the merged version.  compile_reproducer will
    swap in alternatives if compilation fails.

    Returns list of .c file paths in the output directory."""
    finding_out_dir.mkdir(parents=True, exist_ok=True)

    header = textwrap.dedent("""\
        /*
         * Standalone reproducer — build with ASan.
         * Generated by SAILOR (concrete_verify.py).
         */
    """)

    out_c_files = []
    # Only include replay .c files (replay_driver.c) — NOT companion .c files
    # from harness/.  Companion .c files contain agent-sliced source code that
    # would override upstream library symbols, defeating the purpose of concrete
    # verification against the real upstream build.
    all_source_files = list(finding["replay_c_files"])

    for src_file in all_source_files:
        content = src_file.read_text(errors="replace")
        cleaned = strip_sailor_markers(content)
        # Add header to the main driver
        if src_file.name == "replay_driver.c":
            cleaned = header + cleaned
        out_path = finding_out_dir / src_file.name
        # Rename to reproducer.c if it's the driver
        if src_file.name == "replay_driver.c":
            out_path = finding_out_dir / "reproducer.c"
        out_path.write_text(cleaned)
        out_c_files.append(out_path)

    # Copy any .h files (from asan_replay and harness companion)
    h_basenames_seen = set()
    for h_file in finding["replay_h_files"]:
        content = h_file.read_text(errors="replace")
        cleaned = strip_sailor_markers(content)
        (finding_out_dir / h_file.name).write_text(cleaned)
        h_basenames_seen.add(h_file.name)

    for h_file in finding.get("harness_companion_h", []):
        if h_file.name not in h_basenames_seen:
            content = h_file.read_text(errors="replace")
            cleaned = strip_sailor_markers(content)
            (finding_out_dir / h_file.name).write_text(cleaned)
            h_basenames_seen.add(h_file.name)

    # ── Build harness_types.h variants ──
    upstream_types = None
    if upstream_src_dir:
        upstream_types = _generate_upstream_harness_types(upstream_src_dir, asan_build_dir)

    agent_types = None
    if finding["harness_types_h"]:
        content = finding["harness_types_h"].read_text(errors="replace")
        agent_types = strip_sailor_markers(content)

    if upstream_types and agent_types:
        # Save both individual versions for fallback
        (finding_out_dir / "harness_types_upstream.h").write_text(upstream_types)
        (finding_out_dir / "harness_types_agent.h").write_text(agent_types)
        # Primary: merged version (upstream headers + agent custom defs)
        merged = _merge_harness_types(upstream_types, agent_types)
        (finding_out_dir / "harness_types.h").write_text(merged)
    elif upstream_types:
        (finding_out_dir / "harness_types.h").write_text(upstream_types)
    elif agent_types:
        (finding_out_dir / "harness_types.h").write_text(agent_types)

    return out_c_files


# ── Stage 4: Compile & Run Against Upstream ─────────────────────────

def _try_compile(c_file_strs, cflags, inc_flags, upstream_libs, out_bin, compiler):
    """Try compiling with given compiler, then alternate. Returns (ok, stderr).

    upstream_libs can be a single Path, a list of Paths, or None.
    """
    link_flags = ["-lm", "-lpthread", "-lz", "-ldl"]

    # Normalize upstream_libs to a list of strings
    if upstream_libs is None:
        lib_strs = []
    elif isinstance(upstream_libs, list):
        lib_strs = [str(l) for l in upstream_libs]
    else:
        lib_strs = [str(upstream_libs)]

    # Strategy 1: Link against upstream .a libraries
    if lib_strs:
        cmd = [compiler] + cflags + inc_flags + c_file_strs + lib_strs + ["-o", str(out_bin)] + link_flags
        rc, _, stderr, _ = run_cmd(cmd, timeout=60)
        if rc == 0:
            return True, ""
        alt_compiler = "clang" if compiler == "gcc" else "gcc"
        cmd[0] = alt_compiler
        rc, _, stderr, _ = run_cmd(cmd, timeout=60)
        if rc == 0:
            return True, ""

    # Strategy 2: Self-contained (no upstream lib)
    cmd = [compiler] + cflags + inc_flags + c_file_strs + ["-o", str(out_bin), "-lm"]
    rc, _, stderr, _ = run_cmd(cmd, timeout=60)
    if rc == 0:
        return True, ""
    alt_compiler = "clang" if compiler == "gcc" else "gcc"
    cmd[0] = alt_compiler
    rc, _, stderr, _ = run_cmd(cmd, timeout=60)
    if rc == 0:
        return True, ""

    return False, stderr


def compile_reproducer(c_files, upstream_lib, upstream_src_dir, out_bin, compiler, asan_build_dir):
    """Compile the reproducer against the upstream ASan-instrumented library.

    Tries three harness_types.h variants in order:
      1. Merged (upstream headers + agent defs) — default harness_types.h
      2. Agent-original (harness_types_agent.h) — has all custom types
      3. Upstream-only (harness_types_upstream.h) — real types only

    Returns (success, stderr, types_used) where types_used is one of
    "merged", "agent", "upstream", or "" on failure."""

    cflags = ["-fsanitize=address", "-fno-omit-frame-pointer", "-g", "-O0", "-w"]

    # Include paths: upstream source tree's include dirs
    include_dirs = set()
    if upstream_src_dir:
        for inc_dir in ["include", ".", upstream_src_dir.name]:
            candidate = upstream_src_dir / inc_dir
            if candidate.is_dir():
                include_dirs.add(str(candidate))
        # Also add subdirectories that contain headers (bfd/, opcodes/, etc.)
        for sub in upstream_src_dir.iterdir():
            if sub.is_dir() and any(sub.glob("*.h")):
                include_dirs.add(str(sub))
        if asan_build_dir and asan_build_dir != upstream_src_dir and asan_build_dir.is_dir():
            for inc_dir in ["include", ".", "config"]:
                candidate = asan_build_dir / inc_dir
                if candidate.is_dir():
                    include_dirs.add(str(candidate))
            # Also add build subdirectories with generated headers
            for sub in asan_build_dir.iterdir():
                if sub.is_dir() and any(sub.glob("*.h")):
                    include_dirs.add(str(sub))
    # Include the finding output dir for harness_types.h
    if c_files:
        include_dirs.add(str(c_files[0].parent))
    inc_flags = [f"-I{d}" for d in sorted(include_dirs)]
    c_file_strs = [str(f) for f in c_files]

    finding_dir = c_files[0].parent if c_files else None
    harness_types_path = finding_dir / "harness_types.h" if finding_dir else None
    agent_h = finding_dir / "harness_types_agent.h" if finding_dir else None
    upstream_h = finding_dir / "harness_types_upstream.h" if finding_dir else None

    # Build the list of (label, header_path) to try in order
    variants = [("merged", None)]  # None means use current harness_types.h as-is
    if agent_h and agent_h.exists():
        variants.append(("agent", agent_h))
    if upstream_h and upstream_h.exists():
        variants.append(("upstream", upstream_h))

    last_err = ""
    for label, alt_h in variants:
        # Swap in the alternative harness_types.h if needed
        if alt_h and harness_types_path:
            shutil.copy2(alt_h, harness_types_path)

        ok, stderr = _try_compile(c_file_strs, cflags, inc_flags,
                                  upstream_lib, out_bin, compiler)
        if ok:
            return True, "", label
        last_err = stderr

    return False, last_err, ""


def run_reproducer(out_bin, timeout):
    """Run the compiled reproducer with ASan and parse the output."""
    env = dict(os.environ)
    env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1:print_stacktrace=1"
    rc, stdout, stderr, elapsed = run_cmd([str(out_bin)], timeout=timeout, env=env)

    combined = stdout + "\n" + stderr
    asan_info = parse_asan_output(combined)
    asan_info["exit_code"] = rc
    asan_info["elapsed"] = elapsed
    asan_info["output"] = combined[:8000]
    return asan_info


def classify_verdict(original_finding, upstream_asan):
    """Compare original finding with upstream ASan result to produce a verdict."""
    if not upstream_asan.get("asan_triggered"):
        exit_code = upstream_asan.get("exit_code", 0)
        if exit_code == 0:
            return NOT_REPRODUCIBLE
        elif exit_code == -1:  # timeout
            return INCONCLUSIVE
        else:
            return INCONCLUSIVE

    # ASan fired — but check if crash is in the harness itself
    if _is_harness_crash(upstream_asan):
        return HARNESS_ARTIFACT

    # Compare error types
    orig_type = original_finding.get("asan_error_type", "").lower()
    upstream_type = upstream_asan.get("error_type", "").lower()

    if not orig_type or not upstream_type:
        return CONFIRMED  # can't compare types, but ASan fired

    # Normalize: "heap-buffer-overflow" and "global-buffer-overflow" are both buffer overflows
    def normalize_type(t):
        t = t.lower().replace("-", "_")
        if "buffer_overflow" in t:
            return "buffer_overflow"
        if "use_after" in t:
            return "use_after_free"
        return t

    if normalize_type(orig_type) == normalize_type(upstream_type):
        return CONFIRMED
    else:
        return PARTIAL


# ── Stage 5: Generate Reports ───────────────────────────────────────

def generate_build_sh(finding, upstream_libs, upstream_src_dir, compiler, c_files):
    """Generate build.sh instructions for the reproducer."""
    c_names = " ".join(f.name for f in c_files)
    lines = ["#!/bin/bash", "# Build instructions for this reproducer", ""]

    inc_flags = ""
    if upstream_src_dir:
        for d in ["include", "."]:
            if (upstream_src_dir / d).is_dir():
                inc_flags += f" -I{upstream_src_dir / d}"

    # Normalize upstream_libs
    if upstream_libs is None:
        lib_list = []
    elif isinstance(upstream_libs, list):
        lib_list = upstream_libs
    else:
        lib_list = [upstream_libs]

    if lib_list:
        lines.append(f'{compiler} -fsanitize=address -fno-omit-frame-pointer -g -O0 \\')
        lines.append(f'    {c_names} \\')
        for lib in lib_list:
            lines.append(f'    {lib} \\')
        lines.append(f'    -o reproducer_bin -lm -lpthread -lz')
    else:
        lines.append(f'{compiler} -fsanitize=address -fno-omit-frame-pointer -g -O0 \\')
        lines.append(f'    {c_names} \\')
        lines.append(f'    -o reproducer_bin -lm')

    lines.append("")
    lines.append("ASAN_OPTIONS='detect_leaks=0:halt_on_error=1:print_stacktrace=1' ./reproducer_bin")
    return "\n".join(lines)


def generate_report_md(finding, upstream_info, upstream_asan, verdict, build_sh):
    """Generate a markdown bug report."""
    error_type = upstream_asan.get("error_type", finding.get("asan_error_type", "unknown"))
    function = upstream_asan.get("function", finding.get("bug_func", "unknown"))
    file = finding.get("bug_file", "unknown")
    line = finding.get("bug_line", "unknown")
    cwe = finding.get("cwe", "unknown")
    commit_hash = upstream_info.get("commit_hash", "unknown")
    commit_date = upstream_info.get("commit_date", "unknown")
    asan_output = upstream_asan.get("output", "").strip()
    source_context = finding.get("original_context", "")

    report = f"""\
# {error_type} in {function}() — {file}:{line}

## Summary
| Field | Value |
|-------|-------|
| CWE | {cwe} |
| Error Type | {error_type} |
| Function | `{function}()` |
| File | `{file}` |
| Line | {line} |
| Upstream Commit | `{commit_hash[:12]}` ({commit_date}) |
| Verification | **{verdict}** |

## Reproducer
Save as `reproducer.c`, then:
```bash
{build_sh}
```

## ASan Output
```
{asan_output[:4000]}
```

## Source Context
```c
{source_context}
```
"""
    return report


def generate_result_json(finding, upstream_info, upstream_asan, verdict):
    """Generate machine-readable result JSON."""
    return {
        "spec": finding["spec"],
        "verdict": verdict,
        "original_verdict": finding["verdict"],
        "original_asan_type": finding.get("asan_error_type", ""),
        "upstream_asan_type": upstream_asan.get("error_type", ""),
        "upstream_asan_triggered": upstream_asan.get("asan_triggered", False),
        "upstream_asan_function": upstream_asan.get("function", ""),
        "upstream_asan_file": upstream_asan.get("file", ""),
        "upstream_asan_line": upstream_asan.get("line", ""),
        "upstream_exit_code": upstream_asan.get("exit_code", ""),
        "upstream_commit": upstream_info.get("commit_hash", ""),
        "upstream_commit_date": upstream_info.get("commit_date", ""),
        "bug_file": finding.get("bug_file", ""),
        "bug_line": finding.get("bug_line", ""),
        "bug_func": finding.get("bug_func", ""),
        "cwe": finding.get("cwe", ""),
        "timestamp": datetime.now().isoformat(),
    }


# ── Stage 6: OSS-Fuzz Validation ────────────────────────────────────

# Regex to extract concrete data blocks from replay_driver.c
# Groups: (1) variable name, (2) byte count, (3) data array var name, (4) hex values
CONCRETE_BLOCK_RE = re.compile(
    r'/\*\s*replay:\s*concrete\s+values\s+for\s+"([^"]+)"\s+\((\d+)\s+bytes?\)\s*\*/'
    r'\s*static\s+const\s+unsigned\s+char\s+(\w+_data)\[\]\s*=\s*\{([^}]+)\};',
    re.DOTALL,
)

# Regex to match entire concrete blocks for replacement in the harness.
# Matches: { /* replay: ... */ static ... = {...}; memcpy(DEST, NAME_data, ...); };
# Optionally preceded by if (...).
# Groups: (1) variable name, (2) byte count, (3) memcpy target
CONCRETE_BLOCK_FULL_RE = re.compile(
    r'\{\s*/\*\s*replay:\s*concrete\s+values\s+for\s+"([^"]+)"\s+\((\d+)\s+bytes?\)\s*\*/'
    r'\s*static\s+const\s+unsigned\s+char\s+\w+_data\[\]\s*=\s*\{[^}]+\};'
    r'\s*memcpy\(([^,]+),\s*\w+_data,\s*[^;]+;'
    r'\s*\};',
    re.DOTALL,
)

# Regex for direct byte assignments: buf[0] = 'x'; or buf[0] = 0x26; or buf[0] = 38;
# Groups: (1) variable name, (2) index, (3) value (char literal, hex, or decimal)
DIRECT_BYTE_ASSIGN_RE = re.compile(
    r'(\w+)\[(\d+)\]\s*=\s*'
    r'(?:'
    r"'(\\?.)'|"           # char literal: 'x' or '\n'
    r'(0x[0-9a-fA-F]+)|'   # hex literal: 0x2d
    r'(\d+)'               # decimal literal: 45
    r')\s*;'
)


def _parse_char_literal(ch):
    """Parse a C char literal (possibly escaped) to a byte value."""
    escape_map = {
        '\\n': ord('\n'), '\\t': ord('\t'), '\\r': ord('\r'),
        '\\0': 0, '\\\\': ord('\\'), "\\'": ord("'"),
        '\\"': ord('"'), '\\a': 7, '\\b': 8, '\\f': 12, '\\v': 11,
    }
    if ch in escape_map:
        return escape_map[ch]
    if ch.startswith('\\x') and len(ch) == 4:
        return int(ch[2:], 16)
    if ch.startswith('\\') and ch[1:].isdigit():
        return int(ch[1:], 8)
    return ord(ch[0]) if ch else 0


def _extract_direct_assignments(content):
    """Extract groups of sequential direct byte assignments from C source.

    Detects patterns like:
        buf[0] = '&';
        buf[1] = '#';
    or:
        buf[0] = 0x26;
        buf[1] = 0x23;

    Returns list of dicts: [{"name": "buf", "bytes": b'&#', "assignments": [(line_start, line_end), ...]}]
    """
    # Find all direct byte assignments
    assignments = []
    for m in DIRECT_BYTE_ASSIGN_RE.finditer(content):
        var_name = m.group(1)
        index = int(m.group(2))
        char_lit = m.group(3)
        hex_lit = m.group(4)
        dec_lit = m.group(5)

        if char_lit is not None:
            byte_val = _parse_char_literal(char_lit)
        elif hex_lit is not None:
            byte_val = int(hex_lit, 16) & 0xFF
        elif dec_lit is not None:
            byte_val = int(dec_lit) & 0xFF
        else:
            continue

        assignments.append({
            "var": var_name,
            "index": index,
            "byte": byte_val,
            "start": m.start(),
            "end": m.end(),
            "full_match": m.group(0),
        })

    if not assignments:
        return []

    # Group by variable name, keeping only sequential assignments (0, 1, 2, ...)
    from collections import defaultdict
    by_var = defaultdict(list)
    for a in assignments:
        by_var[a["var"]].append(a)

    results = []
    for var_name, var_assigns in by_var.items():
        # Sort by index
        var_assigns.sort(key=lambda x: x["index"])

        # Check for sequential indices starting from 0
        if var_assigns[0]["index"] != 0:
            continue
        sequential = [var_assigns[0]]
        for j in range(1, len(var_assigns)):
            if var_assigns[j]["index"] == sequential[-1]["index"] + 1:
                sequential.append(var_assigns[j])
            else:
                break

        if len(sequential) < 1:
            continue

        data = bytes(a["byte"] for a in sequential)
        results.append({
            "name": var_name,
            "bytes": data,
            "assignments": sequential,
        })

    return results


def extract_seed_corpus(finding, seed_dir):
    """Extract concrete byte arrays from replay_driver.c as seed corpus for libFuzzer.

    Tries two strategies:
    1. Structured concrete blocks: { /* replay: concrete values for "NAME" ... */ ... }
    2. Direct byte assignments: buf[0] = '&'; buf[1] = '#'; ...

    Returns (seed_files, total_bytes, layout) or ([], 0, []) if not fuzzable.
    """
    replay_driver = finding["replay_dir"] / "replay_driver.c"
    content = replay_driver.read_text(errors="replace")

    # Strategy 1: Structured concrete blocks (from KLEE replay)
    matches = list(CONCRETE_BLOCK_RE.finditer(content))

    # Strategy 2: Direct byte assignments (from LLM-crafted drivers)
    direct_groups = _extract_direct_assignments(content) if not matches else []

    if not matches and not direct_groups:
        return [], 0, []

    seed_dir.mkdir(parents=True, exist_ok=True)

    layout = []
    all_bytes = bytearray()
    seed_files = []

    # Process structured blocks
    for i, m in enumerate(matches):
        name = m.group(1)
        hex_str = m.group(4)

        # Parse 0xHH values
        hex_values = re.findall(r'0x([0-9a-fA-F]{2})', hex_str)
        data = bytes(int(h, 16) for h in hex_values)

        layout.append({
            "name": name,
            "offset": len(all_bytes),
            "size": len(data),
            "source": "concrete_block",
        })

        seed_path = seed_dir / f"seed_{i}_{name}.bin"
        seed_path.write_bytes(data)
        seed_files.append(seed_path)

        all_bytes.extend(data)

    # Process direct byte assignments
    for i, group in enumerate(direct_groups):
        name = group["name"]
        data = group["bytes"]

        layout.append({
            "name": name,
            "offset": len(all_bytes),
            "size": len(data),
            "source": "direct_assign",
        })

        idx = len(matches) + i
        seed_path = seed_dir / f"seed_{idx}_{name}.bin"
        seed_path.write_bytes(data)
        seed_files.append(seed_path)

        all_bytes.extend(data)

    # Concatenated seed
    seed_all = seed_dir / "seed_all.bin"
    seed_all.write_bytes(bytes(all_bytes))
    seed_files.insert(0, seed_all)

    # Layout manifest
    with open(seed_dir / "layout.json", "w") as f:
        json.dump(layout, f, indent=2)

    return seed_files, len(all_bytes), layout


def generate_fuzz_harness(finding, finding_out_dir, layout, ossfuzz_dir):
    """Generate a LLVMFuzzerTestOneInput harness from the cleaned reproducer.

    Returns (harness_path, companion_c_files).
    """
    ossfuzz_dir.mkdir(parents=True, exist_ok=True)

    reproducer_path = finding_out_dir / "reproducer.c"
    content = reproducer_path.read_text(errors="replace")

    # Strip any residual klee/SAILOR comments not caught by prepare_reproducer
    content = re.sub(r'/\*\s*klee_\w+\s+removed\s+for\s+replay\s*\*/', '', content)

    # Companion .c files (everything except reproducer.c)
    companion_c_files = [
        f for f in sorted(finding_out_dir.glob("*.c"))
        if f.name != "reproducer.c"
    ]

    # Build offset map from layout
    offset_map = {entry["name"]: entry for entry in layout}
    total_size = sum(e["size"] for e in layout)

    # Replace each concrete block with fuzz_data slicing.
    # The block { /* replay: ... */ static ...; memcpy(DEST, ...); }; becomes
    # { memcpy(DEST, fuzz_data + OFFSET, SIZE); } — preserving any if() guard.
    def replace_block(m):
        name = m.group(1)
        dest = m.group(3).strip()
        entry = offset_map.get(name)
        if entry:
            return f'{{ memcpy({dest}, fuzz_data + {entry["offset"]}, {entry["size"]}); }}'
        return m.group(0)  # no match in layout, keep original

    harness = CONCRETE_BLOCK_FULL_RE.sub(replace_block, content)

    # Replace direct byte assignments (buf[0] = '&'; buf[1] = '#';) with
    # memcpy(buf, fuzz_data + OFFSET, SIZE) for variables in the layout.
    direct_assign_entries = [e for e in layout if e.get("source") == "direct_assign"]
    for entry in direct_assign_entries:
        var = entry["name"]
        size = entry["size"]
        offset = entry["offset"]
        # Build a regex that matches the consecutive assignments: var[0]=...; var[1]=...; ...
        # Replace the entire group with a single memcpy.
        # Use [ \t]* (not \s*) after the semicolon to avoid consuming the next line's indentation.
        assign_lines_re = re.compile(
            r"(?:[ \t]*" + re.escape(var) + r"\[\d+\]\s*=\s*(?:'(?:\\?.)?'|0x[0-9a-fA-F]+|\d+)\s*;[ \t]*\n){" +
            str(size) + r",}",
        )
        def make_memcpy(m, _var=var, _offset=offset, _size=size):
            # Preserve leading indentation from the first line
            leading = re.match(r'^(\s*)', m.group(0))
            indent = leading.group(1) if leading else '    '
            return f'{indent}memcpy({_var}, fuzz_data + {_offset}, {_size});\n'

        harness = assign_lines_re.sub(make_memcpy, harness, count=1)

    # Replace main() with LLVMFuzzerTestOneInput
    harness = re.sub(
        r'int\s+main\s*\([^)]*\)\s*\{',
        f'int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {{\n'
        f'    if (fuzz_size < {total_size}) return 0;',
        harness,
        count=1,
    )

    # Ensure required includes
    for inc in ['#include <stddef.h>', '#include <stdint.h>']:
        if inc not in harness:
            harness = inc + '\n' + harness

    # Copy companion .c and .h files to ossfuzz_dir for self-contained build
    for comp in companion_c_files:
        shutil.copy2(str(comp), str(ossfuzz_dir / comp.name))
    for h_file in finding_out_dir.glob("*.h"):
        shutil.copy2(str(h_file), str(ossfuzz_dir / h_file.name))

    harness_path = ossfuzz_dir / "fuzz_harness.c"
    harness_path.write_text(harness)

    return harness_path, companion_c_files


def run_libfuzzer(harness_path, companion_c_files, seed_dir, upstream_lib,
                  upstream_src_dir, asan_build_dir, fuzz_seconds):
    """Compile fuzz harness with libFuzzer and run against seed corpus.

    Returns a result dict with compiled, asan_triggered, error_type, etc.
    """
    ossfuzz_dir = harness_path.parent
    fuzz_bin = ossfuzz_dir / "fuzz_bin"

    # Build include flags
    include_dirs = set()
    if upstream_src_dir:
        for d in ["include", "."]:
            candidate = upstream_src_dir / d
            if candidate.is_dir():
                include_dirs.add(str(candidate))
        if asan_build_dir and asan_build_dir != upstream_src_dir:
            for d in ["include", ".", "config"]:
                candidate = asan_build_dir / d
                if candidate.is_dir():
                    include_dirs.add(str(candidate))
    # Include finding_out_dir and ossfuzz_dir for companion headers + harness_types.h
    finding_out_dir = ossfuzz_dir.parent
    include_dirs.add(str(finding_out_dir))
    include_dirs.add(str(ossfuzz_dir))
    inc_flags = [f"-I{d}" for d in sorted(include_dirs)]

    # Compile with clang (libFuzzer requires clang)
    cflags = ["-fsanitize=address,fuzzer", "-fno-omit-frame-pointer", "-g", "-O0", "-w"]
    cmd = ["clang"] + cflags + inc_flags + [str(harness_path)]
    cmd.extend(str(f) for f in companion_c_files)
    if upstream_lib:
        if isinstance(upstream_lib, list):
            cmd.extend(str(l) for l in upstream_lib)
        else:
            cmd.append(str(upstream_lib))
    cmd.extend(["-o", str(fuzz_bin), "-lm", "-lpthread", "-lz", "-ldl"])

    rc, stdout, stderr, elapsed = run_cmd(cmd, timeout=60)
    compile_output = stdout + "\n" + stderr

    if rc != 0:
        (ossfuzz_dir / "compile_output.txt").write_text(compile_output[:8000])
        return {"compiled": False, "compile_output": compile_output[:4000]}

    # Run libFuzzer with seed corpus
    env = dict(os.environ)
    env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1:print_stacktrace=1"

    fuzz_cmd = [
        str(fuzz_bin), str(seed_dir),
        f"-max_total_time={fuzz_seconds}",
        "-runs=1000000",
    ]

    rc, stdout, stderr, elapsed = run_cmd(
        fuzz_cmd, timeout=fuzz_seconds + 30, env=env)

    combined = stdout + "\n" + stderr
    asan_info = parse_asan_output(combined)

    (ossfuzz_dir / "fuzz_asan_output.txt").write_text(combined[:16000])

    return {
        "compiled": True,
        "asan_triggered": asan_info.get("asan_triggered", False),
        "error_type": asan_info.get("error_type", ""),
        "function": asan_info.get("function", ""),
        "file": asan_info.get("file", ""),
        "line": asan_info.get("line", ""),
        "exit_code": rc,
        "elapsed": elapsed,
        "output": combined[:8000],
    }


def classify_ossfuzz_verdict(finding, fuzz_result):
    """Classify OSS-Fuzz validation result into a verdict."""
    if not fuzz_result.get("compiled"):
        return FUZZ_BUILD_FAIL

    if not fuzz_result.get("asan_triggered"):
        return FUZZ_NO_CRASH

    # Compare error types
    orig_type = finding.get("asan_error_type", "").lower()
    fuzz_type = fuzz_result.get("error_type", "").lower()

    if not orig_type or not fuzz_type:
        return FUZZ_REPRODUCED

    def normalize_type(t):
        t = t.lower().replace("-", "_")
        if "buffer_overflow" in t:
            return "buffer_overflow"
        if "use_after" in t:
            return "use_after_free"
        return t

    if normalize_type(orig_type) == normalize_type(fuzz_type):
        return FUZZ_REPRODUCED
    return FUZZ_PARTIAL


def write_summary_files(all_results, output_dir):
    """Write aggregate verification_summary.tsv and .json."""
    has_ossfuzz = any(r.get("ossfuzz_verdict") for r in all_results)

    # TSV
    tsv_path = output_dir / "verification_summary.tsv"
    fieldnames = [
        "Spec", "OriginalVerdict", "ConcreteVerdict",
        "OriginalAsanType", "UpstreamAsanType",
        "File", "Line", "Function", "CWE",
    ]
    if has_ossfuzz:
        fieldnames.extend(["FuzzVerdict", "OssFuzzAsanType"])

    with open(tsv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t")
        writer.writeheader()
        for r in all_results:
            row = {
                "Spec": r["spec"],
                "OriginalVerdict": r["original_verdict"],
                "ConcreteVerdict": r["verdict"],
                "OriginalAsanType": r.get("original_asan_type", ""),
                "UpstreamAsanType": r.get("upstream_asan_type", ""),
                "File": r.get("bug_file", ""),
                "Line": r.get("bug_line", ""),
                "Function": r.get("bug_func", ""),
                "CWE": r.get("cwe", ""),
            }
            if has_ossfuzz:
                row["FuzzVerdict"] = r.get("ossfuzz_verdict", "")
                row["OssFuzzAsanType"] = r.get("ossfuzz_asan_type", "")
            writer.writerow(row)
    print(f"  [+] Wrote {tsv_path}")

    # JSON summary with counts
    counts = {}
    for r in all_results:
        v = r["verdict"]
        counts[v] = counts.get(v, 0) + 1

    summary = {
        "total": len(all_results),
        "counts": counts,
        "results": all_results,
        "timestamp": datetime.now().isoformat(),
    }

    if has_ossfuzz:
        ossfuzz_counts = {}
        for r in all_results:
            v = r.get("ossfuzz_verdict", "")
            if v:
                ossfuzz_counts[v] = ossfuzz_counts.get(v, 0) + 1
        summary["ossfuzz_counts"] = ossfuzz_counts

    json_path = output_dir / "verification_summary.json"
    with open(json_path, "w") as f:
        json.dump(summary, f, indent=2, default=str)
    print(f"  [+] Wrote {json_path}")


# ── Main Orchestrator ────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Verify SAILOR findings against upstream source.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent("""\
            Example:
              python3 sailor_engine/concrete_verify.py \\
                --results-dir se_runs/sailor_engine/libxml2_62911_vul \\
                --upstream-url https://gitlab.gnome.org/GNOME/libxml2.git \\
                --configure-args "--with-python=no --without-http --without-ftp"
        """),
    )
    parser.add_argument("--results-dir", required=True, type=Path,
                        help="SAILOR run output directory containing summary.tsv")
    parser.add_argument("--upstream-url", required=True,
                        help="Git URL of the upstream project")
    parser.add_argument("--upstream-branch", default="master",
                        help="Branch to clone (default: master)")
    parser.add_argument("--upstream-commit", default=None,
                        help="Exact commit hash to checkout and verify against "
                             "(default: branch HEAD)")
    parser.add_argument("--upstream-dir", type=Path, default=None,
                        help="Use existing local checkout instead of cloning")
    parser.add_argument("--build-system", choices=["autotools", "cmake", "auto"],
                        default="auto", help="Build system (default: auto-detect)")
    parser.add_argument("--configure-args", default="",
                        help="Extra configure/cmake flags")
    parser.add_argument("--specs", nargs="+", default=None,
                        help="Verify only specific spec stems (default: all TP/LIKELY_TP)")
    parser.add_argument("--skip-build", action="store_true",
                        help="Reuse cached upstream build")
    parser.add_argument("--compiler", choices=["gcc", "clang"], default="gcc",
                        help="Compiler for upstream build (default: gcc)")
    parser.add_argument("--timeout", type=int, default=10,
                        help="Per-reproducer run timeout in seconds (default: 10)")
    parser.add_argument("--output-dir", type=Path, default=None,
                        help="Output directory (default: artifacts/<project>/verify_data)")
    parser.add_argument("--ossfuzz-validate", action="store_true",
                        help="Run OSS-Fuzz/libFuzzer validation after upstream verification")
    parser.add_argument("--fuzz-seconds", type=int, default=30,
                        help="libFuzzer run duration per finding in seconds (default: 30)")
    parser.add_argument("--tp-summary", type=Path, default=None,
                        help="Path to summary_tp.tsv with dedup info; "
                             "only verify non-duplicate (unique) specs")
    args = parser.parse_args()

    results_dir = args.results_dir.resolve()
    if not results_dir.is_dir():
        print(f"[ERROR] Results directory not found: {results_dir}")
        sys.exit(1)

    # Derive output dir from results dir name if not specified
    project_name = results_dir.name
    if args.output_dir:
        output_dir = args.output_dir.resolve()
    else:
        output_dir = Path("artifacts") / project_name / "verify_data"
    output_dir = output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    upstream_base = output_dir / "upstream"
    findings_base = output_dir / "findings"
    findings_base.mkdir(parents=True, exist_ok=True)

    print(f"=" * 60)
    print(f"SAILOR Upstream Verification")
    print(f"  Results:  {results_dir}")
    print(f"  Upstream: {args.upstream_url}")
    print(f"  Branch:   {args.upstream_branch}")
    print(f"  Output:   {output_dir}")
    print(f"=" * 60)

    # ── Stage 1: Scan findings ──
    print(f"\n[Stage 1] Scanning findings...")
    spec_filter = set(args.specs) if args.specs else None
    findings = scan_findings(results_dir, spec_filter)
    if not findings:
        print("[i] No findings to verify. Exiting.")
        sys.exit(0)

    # Filter to unique (non-duplicate) specs if --tp-summary provided
    if args.tp_summary:
        tp_summary_path = args.tp_summary.resolve()
        if tp_summary_path.is_file():
            unique_specs = set()
            with open(tp_summary_path, newline="") as f:
                reader = csv.DictReader(f, delimiter="\t")
                for row in reader:
                    if row.get("IsDuplicate") != "true":
                        unique_specs.add(row.get("Spec", ""))
            unique_specs.discard("")
            before = len(findings)
            findings = [f for f in findings if f["spec"] in unique_specs]
            print(f"  [dedup] Filtered {before} → {len(findings)} unique specs via {tp_summary_path.name}")
        else:
            print(f"  [WARN] --tp-summary file not found: {tp_summary_path}")

    if not findings:
        print("[i] No findings to verify after dedup filtering. Exiting.")
        sys.exit(0)
    print(f"  Found {len(findings)} finding(s) to verify.\n")

    # ── Stage 2: Clone & build upstream ──
    print(f"[Stage 2] Building upstream with ASan...")
    upstream_src_dir = args.upstream_dir if args.upstream_dir else (upstream_base / "src")
    upstream_info = clone_upstream(args.upstream_url, args.upstream_branch,
                                  upstream_src_dir, args.upstream_commit)
    if upstream_info is None:
        print("[ERROR] Failed to clone upstream.")
        # Write all results as UPSTREAM_BUILD_FAIL
        all_results = []
        for f in findings:
            all_results.append(generate_result_json(f, {}, {}, UPSTREAM_BUILD_FAIL))
        write_summary_files(all_results, output_dir)
        sys.exit(1)
    print(f"  Upstream HEAD: {upstream_info['commit_hash'][:12]} ({upstream_info['commit_date']})")

    _verified_hash = upstream_info.get("commit_hash", "")

    upstream_lib = None
    asan_build_dir = upstream_base / "asan_build"
    if args.skip_build:
        # Try to find existing libraries
        all_libs = []
        for search_dir in [asan_build_dir, upstream_src_dir]:
            if not search_dir.exists():
                continue
            found = list(search_dir.rglob("*.a"))
            found = [f for f in found if "CMakeFiles" not in str(f)
                     and "testsuite" not in str(f)
                     and "/noasan/" not in str(f)]
            all_libs.extend(found)
        if all_libs:
            # Deduplicate by name, prefer .libs/
            seen = {}
            for lib in all_libs:
                if lib.name not in seen or ".libs" in str(lib):
                    seen[lib.name] = lib
            upstream_lib = sorted(seen.values(), key=lambda p: p.name)
            for lib in upstream_lib:
                print(f"  [build] Reusing cached library: {lib}")
    else:
        build_system = args.build_system
        if build_system == "auto":
            build_system = detect_build_system(upstream_src_dir)
            if not build_system:
                print(f"  [WARN] Could not detect build system in {upstream_src_dir}")
            else:
                print(f"  [build] Detected build system: {build_system}")

        if build_system:
            upstream_lib = build_upstream_asan(
                upstream_src_dir, asan_build_dir, build_system,
                args.configure_args, args.compiler)

    if upstream_lib is None:
        print(f"  [WARN] No upstream library found; will try self-contained compilation.")
    print()

    # ── Stages 3-5: Per-finding verification ──
    all_results = []
    for i, finding in enumerate(findings, 1):
        spec = finding["spec"]
        print(f"[Finding {i}/{len(findings)}] {spec}")

        finding_out_dir = findings_base / spec

        # Stage 3: Strip markers and prepare reproducer
        print(f"  [Stage 3] Preparing reproducer...")
        reproducer_c_files = prepare_reproducer(finding, finding_out_dir,
                                                upstream_src_dir=upstream_src_dir,
                                                asan_build_dir=asan_build_dir)

        # Stage 4: Compile & run
        print(f"  [Stage 4] Compiling against upstream...")
        out_bin = finding_out_dir / "reproducer_bin"
        compile_ok, compile_err, types_used = compile_reproducer(
            reproducer_c_files, upstream_lib, upstream_src_dir,
            out_bin, args.compiler, asan_build_dir)

        if not compile_ok:
            print(f"  [!] Compilation failed: {compile_err[:200]}")
            verdict = REPRODUCER_BUILD_FAIL
            upstream_asan = {"output": compile_err[:2000]}
        else:
            if types_used and types_used != "merged":
                print(f"  [i] Compiled with {types_used} harness_types.h (merged failed)")
            print(f"  [Stage 4] Running reproducer (timeout={args.timeout}s)...")
            upstream_asan = run_reproducer(out_bin, args.timeout)

            if upstream_asan.get("asan_triggered"):
                print(f"  [!] ASan triggered: {upstream_asan.get('error_type', '?')}")
            else:
                print(f"  [i] No ASan error (exit={upstream_asan.get('exit_code', '?')})")

            verdict = classify_verdict(finding, upstream_asan)

        print(f"  --> Verdict: {verdict}")

        # Stage 5: Generate reports
        # Save ASan output
        asan_out_path = finding_out_dir / "upstream_asan_output.txt"
        asan_out_path.write_text(upstream_asan.get("output", ""))

        # result.json
        result = generate_result_json(finding, upstream_info, upstream_asan, verdict)
        with open(finding_out_dir / "result.json", "w") as f:
            json.dump(result, f, indent=2, default=str)

        # build.sh
        build_sh = generate_build_sh(finding, upstream_lib, upstream_src_dir,
                                     args.compiler, reproducer_c_files)
        (finding_out_dir / "build.sh").write_text(build_sh)
        os.chmod(finding_out_dir / "build.sh", 0o755)

        # REPORT.md
        report_md = generate_report_md(finding, upstream_info, upstream_asan, verdict, build_sh)
        (finding_out_dir / "REPORT.md").write_text(report_md)

        all_results.append(result)
        print()

    # ── Stage 6: OSS-Fuzz validation (optional) ──
    if args.ossfuzz_validate:
        print(f"[Stage 6] OSS-Fuzz validation (fuzz_seconds={args.fuzz_seconds})...")
        for i, finding in enumerate(findings, 1):
            spec = finding["spec"]
            finding_out_dir = findings_base / spec
            ossfuzz_dir = finding_out_dir / "ossfuzz"

            print(f"  [{i}/{len(findings)}] {spec}")

            # 6a: Extract seed corpus from original replay_driver.c
            seeds, total_bytes, layout = extract_seed_corpus(
                finding, ossfuzz_dir / "seed_corpus")
            if not seeds:
                print(f"    NOT_FUZZABLE — no concrete byte arrays")
                all_results[i - 1]["ossfuzz_verdict"] = NOT_FUZZABLE
                ossfuzz_dir.mkdir(parents=True, exist_ok=True)
                with open(ossfuzz_dir / "ossfuzz_result.json", "w") as f:
                    json.dump({"verdict": NOT_FUZZABLE}, f, indent=2)
                continue

            print(f"    Extracted {len(seeds) - 1} input(s), {total_bytes} bytes total")

            # 6b: Generate fuzz harness
            harness_path, companion_c_files = generate_fuzz_harness(
                finding, finding_out_dir, layout, ossfuzz_dir)
            print(f"    Generated {harness_path.name}")

            # 6c: Compile and run libFuzzer
            print(f"    Running libFuzzer ({args.fuzz_seconds}s)...")
            fuzz_result = run_libfuzzer(
                harness_path, companion_c_files,
                ossfuzz_dir / "seed_corpus",
                upstream_lib, upstream_src_dir, asan_build_dir,
                args.fuzz_seconds)

            # 6d: Classify verdict
            ossfuzz_verdict = classify_ossfuzz_verdict(finding, fuzz_result)
            print(f"    OSS-Fuzz verdict: {ossfuzz_verdict}")

            all_results[i - 1]["ossfuzz_verdict"] = ossfuzz_verdict
            all_results[i - 1]["ossfuzz_asan_type"] = fuzz_result.get("error_type", "")

            # Save per-finding OSS-Fuzz result
            ossfuzz_result = {
                "verdict": ossfuzz_verdict,
                "asan_triggered": fuzz_result.get("asan_triggered", False),
                "error_type": fuzz_result.get("error_type", ""),
                "compiled": fuzz_result.get("compiled", False),
                "total_seed_bytes": total_bytes,
                "fuzz_seconds": args.fuzz_seconds,
            }
            with open(ossfuzz_dir / "ossfuzz_result.json", "w") as f:
                json.dump(ossfuzz_result, f, indent=2)

        print()

    # ── Write aggregate summaries ──
    print(f"[Summary] Writing aggregate results...")
    write_summary_files(all_results, output_dir)

    # Print final summary
    counts = {}
    for r in all_results:
        v = r["verdict"]
        counts[v] = counts.get(v, 0) + 1

    print(f"\n{'=' * 60}")
    print(f"Verification complete: {len(all_results)} finding(s)")
    for verdict_name in [CONFIRMED, PARTIAL, NOT_REPRODUCIBLE, REPRODUCER_BUILD_FAIL, UPSTREAM_BUILD_FAIL, INCONCLUSIVE]:
        c = counts.get(verdict_name, 0)
        if c > 0:
            print(f"  {verdict_name}: {c}")

    if any(r.get("ossfuzz_verdict") for r in all_results):
        ossfuzz_counts = {}
        for r in all_results:
            v = r.get("ossfuzz_verdict", "")
            if v:
                ossfuzz_counts[v] = ossfuzz_counts.get(v, 0) + 1
        print(f"\n  OSS-Fuzz validation:")
        for v_name in [FUZZ_REPRODUCED, FUZZ_PARTIAL, FUZZ_NO_CRASH, FUZZ_BUILD_FAIL, NOT_FUZZABLE]:
            c = ossfuzz_counts.get(v_name, 0)
            if c > 0:
                print(f"    {v_name}: {c}")

    print(f"  Output: {output_dir}")
    print(f"{'=' * 60}")


if __name__ == "__main__":
    main()
