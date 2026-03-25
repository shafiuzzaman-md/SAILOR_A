#!/usr/bin/env python3
"""Run fuzz reproduction for verified findings using replay_driver.c artifacts.

Transforms replay_driver.c (with concrete memcpy blocks) into a libFuzzer
harness, compiles against upstream .a, and runs for --fuzz-seconds.

Usage:
    python3 scripts/fuzz_from_replay.py \
        --findings-dir export/verified_artifacts/libselinux_ca10fc4_vul \
        --upstream-libs dataset/.../libsepol.a dataset/.../libselinux.a \
        --include-dirs dataset/.../include \
        --src-root dataset/... \
        --output-dir artifacts/libselinux_ca10fc4_vul/ossfuzz_artifacts \
        --extra-libs "-lm -lpthread" \
        --fuzz-seconds 30 \
        --jobs 8
"""

import argparse
import csv
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed


def run_cmd(cmd, timeout=60, env=None):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, env=env)
        return r.returncode, r.stdout, r.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "timeout"
    except Exception as e:
        return -1, "", str(e)


def transform_to_fuzz_harness(replay_src):
    """Transform replay_driver.c into a libFuzzer harness.

    Replaces concrete memcpy blocks with fuzz_data slicing,
    and transforms main() into LLVMFuzzerTestOneInput.
    """
    # Find all concrete data blocks:
    # { static const unsigned char NAME[] = {0x..., ...}; memcpy(DEST, NAME, ...); };
    # Use [^;]+ for the memcpy size arg to handle nested parens like (x < sizeof(y)) ? x : sizeof(y)
    block_re = re.compile(
        r'\{\s*static\s+const\s+unsigned\s+char\s+(\w+)\[\]\s*=\s*'
        r'\{([^}]*)\}\s*;\s*'
        r'memcpy\((\w+),\s*\1,\s*[^;]+\)\s*;\s*\}',
        re.DOTALL
    )

    # Fallback: match without the outer braces
    block_re2 = re.compile(
        r'static\s+const\s+unsigned\s+char\s+(\w+)\[\]\s*=\s*'
        r'\{([^}]*)\}\s*;\s*'
        r'memcpy\((\w+),\s*\1,\s*[^;]+\)\s*;',
        re.DOTALL
    )

    # Collect all blocks with their sizes and destinations
    blocks = []
    for m in block_re.finditer(replay_src):
        data_bytes = m.group(2).strip()
        size = len([x.strip() for x in data_bytes.split(',') if x.strip()])
        blocks.append({
            'match': m,
            'name': m.group(1),
            'dest': m.group(3),
            'size': size,
            'start': m.start(),
            'end': m.end(),
        })

    if not blocks:
        for m in block_re2.finditer(replay_src):
            data_bytes = m.group(2).strip()
            size = len([x.strip() for x in data_bytes.split(',') if x.strip()])
            blocks.append({
                'match': m,
                'name': m.group(1),
                'dest': m.group(3),
                'size': size,
                'start': m.start(),
                'end': m.end(),
            })

    if not blocks:
        return None, 0  # Can't fuzzify

    # Compute offsets
    offset = 0
    for b in blocks:
        b['offset'] = offset
        offset += b['size']
    total_size = offset

    # Replace blocks with fuzz_data slicing (reverse order to preserve positions)
    result = replay_src
    for b in reversed(blocks):
        replacement = f'{{ memcpy({b["dest"]}, fuzz_data + {b["offset"]}, {b["size"]}); }}'
        result = result[:b['start']] + replacement + result[b['end']:]

    # Replace main() with LLVMFuzzerTestOneInput
    result = re.sub(
        r'int\s+main\s*\([^)]*\)\s*\{',
        f'int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {{\n'
        f'    if (fuzz_size < {total_size}) return 0;',
        result,
        count=1,
    )

    # Add required includes
    for inc in ['#include <stddef.h>', '#include <stdint.h>']:
        if inc not in result:
            result = inc + '\n' + result

    return result, total_size


def extract_seed(replay_src, total_size):
    """Extract concrete values from replay_driver.c as seed corpus."""
    block_re = re.compile(
        r'static\s+const\s+unsigned\s+char\s+\w+\[\]\s*=\s*\{([^}]*)\}',
        re.DOTALL
    )

    seed_bytes = bytearray()
    for m in block_re.finditer(replay_src):
        data = m.group(1).strip()
        for val in data.split(','):
            val = val.strip()
            if val:
                try:
                    seed_bytes.append(int(val, 0) & 0xFF)
                except ValueError:
                    seed_bytes.append(0)

    # Pad or truncate to total_size
    if len(seed_bytes) < total_size:
        seed_bytes.extend(b'\x00' * (total_size - len(seed_bytes)))
    return bytes(seed_bytes[:total_size])


def fuzz_one_finding(args):
    """Fuzz a single finding. Returns result dict."""
    spec_name, finding_dir, upstream_libs, include_dirs, src_root, \
        extra_libs, fuzz_seconds, output_dir = args

    asan_real = finding_dir / "asan_real"
    replay = asan_real / "replay_driver.c"
    if not replay.exists():
        return {"spec": spec_name, "status": "NO_REPLAY", "detail": ""}

    replay_src = replay.read_text(errors="replace")

    # Transform to fuzz harness
    harness_src, total_size = transform_to_fuzz_harness(replay_src)
    if harness_src is None:
        return {"spec": spec_name, "status": "NOT_FUZZABLE", "detail": "no concrete blocks"}

    # Set up output dir
    spec_out = output_dir / spec_name
    spec_out.mkdir(parents=True, exist_ok=True)

    # Write harness
    harness_path = spec_out / "fuzz_harness.c"
    harness_path.write_text(harness_src)

    # Copy companion files
    for f in asan_real.glob("*.c"):
        if f.name != "replay_driver.c":
            shutil.copy2(str(f), str(spec_out / f.name))
    for f in asan_real.glob("*.h"):
        shutil.copy2(str(f), str(spec_out / f.name))

    # Create seed corpus
    seed_dir = spec_out / "seed_corpus"
    seed_dir.mkdir(exist_ok=True)
    seed_data = extract_seed(replay_src, total_size)
    (seed_dir / "seed0").write_bytes(seed_data)

    # Compile
    companion_c = [str(f) for f in spec_out.glob("*.c") if f.name != "fuzz_harness.c"]
    inc_flags = []
    for d in include_dirs:
        inc_flags.extend(["-I", d])
    inc_flags.extend(["-I", str(spec_out)])

    fuzz_bin = spec_out / "fuzz_bin"
    cmd = (["clang", "-fsanitize=address,fuzzer", "-fno-omit-frame-pointer",
            "-g", "-O0", "-w"] +
           inc_flags + [str(harness_path)] + companion_c +
           [str(l) for l in upstream_libs] +
           ["-o", str(fuzz_bin)] + extra_libs)

    rc, stdout, stderr = run_cmd(cmd, timeout=60)
    if rc != 0:
        (spec_out / "compile_error.txt").write_text(stderr[:8000])
        return {"spec": spec_name, "status": "BUILD_FAIL", "detail": stderr[:200]}

    # Run libFuzzer
    env = dict(os.environ)
    env["ASAN_OPTIONS"] = "detect_leaks=0:halt_on_error=1:print_stacktrace=1"
    fuzz_cmd = [str(fuzz_bin), str(seed_dir),
                f"-max_total_time={fuzz_seconds}", "-runs=1000000"]

    rc, stdout, stderr = run_cmd(fuzz_cmd, timeout=fuzz_seconds + 30, env=env)
    combined = stdout + "\n" + stderr

    (spec_out / "fuzz_asan_output.txt").write_text(combined[:16000])

    # Check for crash
    if "ERROR: AddressSanitizer:" in combined or "SEGV" in combined:
        # Extract crash info
        error_type = ""
        for pattern in ["heap-buffer-overflow", "stack-buffer-overflow",
                        "heap-use-after-free", "double-free", "SEGV",
                        "global-buffer-overflow", "stack-use-after-return"]:
            if pattern in combined:
                error_type = pattern
                break

        # Find crash location in library code
        crash_file, crash_line, crash_func = "", "", ""
        for line in combined.split('\n'):
            m = re.search(r'#\d+\s+\S+\s+in\s+(\S+)\s+(\S+):(\d+)', line)
            if m:
                func, fpath, fline = m.group(1), m.group(2), m.group(3)
                if any(skip in fpath for skip in
                       ['fuzz_harness', 'smart_stubs', 'stubs', 'harness_types',
                        '__asan', '__interceptor', 'libsanitizer', '_start', 'libc_start']):
                    continue
                crash_func = func
                crash_file = fpath
                crash_line = fline
                break

        return {
            "spec": spec_name, "status": "FUZZ_REPRODUCED",
            "error_type": error_type,
            "crash_file": crash_file, "crash_line": crash_line,
            "crash_func": crash_func,
        }
    else:
        return {"spec": spec_name, "status": "FUZZ_NO_CRASH", "detail": ""}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--findings-dir", required=True, type=Path)
    parser.add_argument("--upstream-libs", required=True, nargs='+', type=Path)
    parser.add_argument("--include-dirs", default="", type=str)
    parser.add_argument("--src-root", default="", type=str)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--extra-libs", default="-lm -lpthread", type=str)
    parser.add_argument("--fuzz-seconds", type=int, default=30)
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)

    include_dirs = [d.strip() for d in args.include_dirs.split(",") if d.strip()]
    extra_libs = args.extra_libs.split()

    # Find all findings
    findings = sorted(args.findings_dir.iterdir())
    findings = [f for f in findings if f.is_dir() and (f / "asan_real").is_dir()]

    print(f"Found {len(findings)} findings to fuzz")

    tasks = []
    for f in findings:
        tasks.append((
            f.name, f, args.upstream_libs, include_dirs,
            args.src_root, extra_libs, args.fuzz_seconds, args.output_dir
        ))

    results = []
    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        futures = {executor.submit(fuzz_one_finding, t): t[0] for t in tasks}
        for future in as_completed(futures):
            spec = futures[future]
            try:
                result = future.result()
                results.append(result)
                status = result['status']
                if status == 'FUZZ_REPRODUCED':
                    print(f"  ✓ {spec}: REPRODUCED ({result.get('error_type','')})")
                elif status == 'BUILD_FAIL':
                    print(f"  ✗ {spec}: BUILD_FAIL")
                elif status == 'NOT_FUZZABLE':
                    print(f"  - {spec}: NOT_FUZZABLE")
                else:
                    print(f"  · {spec}: {status}")
            except Exception as e:
                results.append({"spec": spec, "status": "ERROR", "detail": str(e)})
                print(f"  ! {spec}: ERROR ({e})")

    # Write summary
    from collections import Counter
    status_counts = Counter(r['status'] for r in results)

    summary_path = args.output_dir / "fuzz_summary.tsv"
    with open(summary_path, 'w', newline='') as f:
        writer = csv.writer(f, delimiter='\t')
        writer.writerow(['Spec', 'FuzzVerdict', 'ErrorType', 'CrashFile', 'CrashLine', 'CrashFunc'])
        for r in sorted(results, key=lambda x: x['spec']):
            writer.writerow([
                r['spec'], r['status'],
                r.get('error_type', ''), r.get('crash_file', ''),
                r.get('crash_line', ''), r.get('crash_func', ''),
            ])

    print(f"\n=== FUZZ SUMMARY ===")
    print(f"Total: {len(results)}")
    for status, count in status_counts.most_common():
        print(f"  {status}: {count}")
    print(f"Saved to {summary_path}")


if __name__ == "__main__":
    main()
