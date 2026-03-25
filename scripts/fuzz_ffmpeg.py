#!/usr/bin/env python3
"""Fuzz reproduction for FFmpeg v2-batch format artifacts.

These use combined reproducer.c (driver + stubs + klee_make_symbolic)
plus sliced source in harness/ subdirectory.
"""

import csv
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed


FFMPEG_ROOT = Path("dataset/f46e5144/ffmpeg_f46e5144_vul")
FFMPEG_LIBS = [
    FFMPEG_ROOT / "libavcodec/libavcodec.a",
    FFMPEG_ROOT / "libavformat/libavformat.a",
    FFMPEG_ROOT / "libavutil/libavutil.a",
    FFMPEG_ROOT / "libswresample/libswresample.a",
    FFMPEG_ROOT / "libswscale/libswscale.a",
    FFMPEG_ROOT / "libavfilter/libavfilter.a",
]
INCLUDE_DIRS = [
    str(FFMPEG_ROOT),
    str(FFMPEG_ROOT / "libavcodec"),
    str(FFMPEG_ROOT / "libavformat"),
    str(FFMPEG_ROOT / "libavutil"),
    "/tmp",  # for dummy klee/klee.h
]


def run_cmd(cmd, timeout=60, env=None):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, env=env)
        return r.returncode, r.stdout, r.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "timeout"
    except Exception as e:
        return -1, "", str(e)


def convert_reproducer(src):
    """Convert klee-based reproducer to libFuzzer harness."""
    result = re.sub(r'#include\s*<klee/klee\.h>\s*\n?', '', src)

    # Find klee_make_symbolic calls
    klee_re = re.compile(r'klee_make_symbolic\(([^,]+),\s*([^,]+),\s*"[^"]*"\)\s*;')
    blocks = list(klee_re.finditer(result))

    if not blocks:
        return None

    # Replace with fuzz_data memcpy - use cumulative offset
    # We keep the size expressions as-is since they're C variables
    replacements = []
    offset_expr = "0"
    for i, m in enumerate(blocks):
        ptr = m.group(1).strip()
        size_expr = m.group(2).strip()
        replacement = f'memcpy({ptr}, fuzz_data + ({offset_expr}), {size_expr});'
        replacements.append((m.start(), m.end(), replacement))
        if i == 0:
            offset_expr = size_expr
        else:
            offset_expr = f"{offset_expr} + {size_expr}"

    # Apply replacements in reverse
    for start, end, repl in reversed(replacements):
        result = result[:start] + repl + result[end:]

    # Remove klee_assume
    result = re.sub(r'klee_assume\([^;]+\)\s*;', '', result)

    # Replace main() with LLVMFuzzerTestOneInput
    result = re.sub(
        r'int\s+main\s*\([^)]*\)\s*\{',
        'int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {\n'
        '    if (fuzz_size < 64) return 0;',
        result, count=1
    )

    for inc in ['#include <stddef.h>', '#include <stdint.h>']:
        if inc not in result:
            result = inc + '\n' + result

    return result


def fuzz_one(args):
    spec_name, spec_dir, output_dir, fuzz_seconds = args

    reproducer = spec_dir / "reproducer.c"
    if not reproducer.exists():
        return {"spec": spec_name, "status": "NO_REPRODUCER"}

    src = reproducer.read_text(errors="replace")
    harness_src = convert_reproducer(src)
    if not harness_src:
        return {"spec": spec_name, "status": "NOT_FUZZABLE"}

    spec_out = output_dir / spec_name
    spec_out.mkdir(parents=True, exist_ok=True)

    harness_path = spec_out / "fuzz_harness.c"
    harness_path.write_text(harness_src)

    # Copy harness_types.h
    for h in spec_dir.glob("*.h"):
        shutil.copy2(str(h), str(spec_out / h.name))

    # Find sliced source in harness/
    harness_dir = spec_dir / "harness"
    sliced_c = []
    if harness_dir.is_dir():
        for f in harness_dir.glob("*.c"):
            if f.name not in ("driver.c", "smart_stubs.c"):
                shutil.copy2(str(f), str(spec_out / f.name))
                sliced_c.append(str(spec_out / f.name))
        for h in harness_dir.glob("*.h"):
            if not (spec_out / h.name).exists():
                shutil.copy2(str(h), str(spec_out / h.name))

    # Create seed
    seed_dir = spec_out / "seed_corpus"
    seed_dir.mkdir(exist_ok=True)
    (seed_dir / "seed0").write_bytes(b'\x00' * 64)

    # Compile
    inc_flags = []
    for d in INCLUDE_DIRS:
        inc_flags.extend(["-I", d])
    inc_flags.extend(["-I", str(spec_out)])
    if harness_dir.is_dir():
        inc_flags.extend(["-I", str(harness_dir)])

    fuzz_bin = spec_out / "fuzz_bin"
    cmd = (["clang", "-fsanitize=address,fuzzer", "-fno-omit-frame-pointer",
            "-g", "-O0", "-w"] + inc_flags +
           [str(harness_path)] + sliced_c +
           [str(l) for l in FFMPEG_LIBS] +
           ["-o", str(fuzz_bin), "-lm", "-lpthread", "-ldl", "-lz"])

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

    if "ERROR: AddressSanitizer:" in combined or "SEGV" in combined or "ABORTING" in combined:
        error_type = ""
        for p in ["heap-buffer-overflow", "stack-buffer-overflow",
                   "heap-use-after-free", "double-free", "SEGV",
                   "global-buffer-overflow"]:
            if p in combined:
                error_type = p
                break

        crash_file, crash_line, crash_func = "", "", ""
        for line in combined.split('\n'):
            m = re.search(r'#\d+\s+\S+\s+in\s+(\S+)\s+(\S+):(\d+)', line)
            if m:
                func, fpath, fline = m.group(1), m.group(2), m.group(3)
                if any(s in fpath for s in ['fuzz_harness', 'stubs', 'harness_types',
                                            '__asan', '__interceptor', 'libsanitizer']):
                    continue
                crash_func, crash_file, crash_line = func, fpath, fline
                break

        return {
            "spec": spec_name, "status": "FUZZ_REPRODUCED",
            "error_type": error_type,
            "crash_file": crash_file, "crash_line": crash_line,
            "crash_func": crash_func,
        }
    else:
        return {"spec": spec_name, "status": "FUZZ_NO_CRASH"}


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--findings-dir", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--fuzz-seconds", type=int, default=30)
    parser.add_argument("--jobs", type=int, default=8)
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)

    findings = sorted(d for d in args.findings_dir.iterdir()
                      if d.is_dir() and (d / "reproducer.c").exists())
    print(f"Found {len(findings)} findings to fuzz")

    tasks = [(f.name, f, args.output_dir, args.fuzz_seconds) for f in findings]

    results = []
    with ProcessPoolExecutor(max_workers=args.jobs) as executor:
        futures = {executor.submit(fuzz_one, t): t[0] for t in tasks}
        for future in as_completed(futures):
            spec = futures[future]
            try:
                result = future.result()
                results.append(result)
                s = result['status']
                if s == 'FUZZ_REPRODUCED':
                    print(f"  ✓ {spec}: REPRODUCED ({result.get('error_type','')})")
                elif s == 'BUILD_FAIL':
                    print(f"  ✗ {spec}: BUILD_FAIL")
                else:
                    print(f"  · {spec}: {s}")
            except Exception as e:
                results.append({"spec": spec, "status": "ERROR"})
                print(f"  ! {spec}: ERROR ({e})")

    # Summary
    summary_path = args.output_dir / "fuzz_summary.tsv"
    with open(summary_path, 'w', newline='') as f:
        writer = csv.writer(f, delimiter='\t')
        writer.writerow(['Spec', 'FuzzVerdict', 'ErrorType', 'CrashFile', 'CrashLine', 'CrashFunc'])
        for r in sorted(results, key=lambda x: x['spec']):
            writer.writerow([r['spec'], r['status'],
                             r.get('error_type', ''), r.get('crash_file', ''),
                             r.get('crash_line', ''), r.get('crash_func', '')])

    from collections import Counter
    counts = Counter(r['status'] for r in results)
    print(f"\n=== FUZZ SUMMARY ===")
    print(f"Total: {len(results)}")
    for s, c in counts.most_common():
        print(f"  {s}: {c}")


if __name__ == "__main__":
    main()
