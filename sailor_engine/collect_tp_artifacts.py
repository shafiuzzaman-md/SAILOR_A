#!/usr/bin/env python3
"""
collect_tp_artifacts.py

After a SAILOR run, collect artifacts from SE_DETECTED specs into a clean
tp_artifacts/ directory for sharing and review.

Copies: bug_report.json, run_report.json, report.json,
        harness/ (*.c, *.h, line_map.json -- skips *.bc),
        asan_replay/replay_driver.c,
        logs/klee_*.log, logs/klee_run_*/test*.ktest, logs/klee_run_*/test*.err

Skips: src_copy/, sa_context/, turn_*_raw.txt, execution.log,
       asan_replay_*/asan_real/, *.bc, KLEE internals (assembly.ll, *.istats, etc.)
"""

import argparse
import csv
import json
import os
import shutil
import sys
from collections import defaultdict
from pathlib import Path


def copy_top_level_jsons(spec_src: Path, spec_dst: Path):
    """Copy bug_report.json, run_report.json, report.json."""
    for name in ("bug_report.json", "run_report.json", "report.json"):
        src = spec_src / name
        if src.exists():
            shutil.copy2(src, spec_dst / name)


def copy_harness(spec_src: Path, spec_dst: Path):
    """Copy harness/*.c, *.h, line_map.json -- skip *.bc."""
    harness_src = spec_src / "harness"
    if not harness_src.is_dir():
        return
    harness_dst = spec_dst / "harness"
    harness_dst.mkdir(parents=True, exist_ok=True)
    for f in harness_src.iterdir():
        if not f.is_file():
            continue
        if f.suffix == ".bc":
            continue
        if f.suffix in (".c", ".h") or f.name == "line_map.json":
            shutil.copy2(f, harness_dst / f.name)


def copy_asan_replay(spec_src: Path, spec_dst: Path):
    """Copy asan_replay/replay_driver.c only."""
    replay_src = spec_src / "asan_replay" / "replay_driver.c"
    if replay_src.exists():
        replay_dst = spec_dst / "asan_replay"
        replay_dst.mkdir(parents=True, exist_ok=True)
        shutil.copy2(replay_src, replay_dst / "replay_driver.c")


def copy_logs(spec_src: Path, spec_dst: Path):
    """Copy logs/klee_*.log and logs/klee_run_*/test*.{ktest,err}."""
    logs_src = spec_src / "logs"
    if not logs_src.is_dir():
        return
    logs_dst = spec_dst / "logs"
    logs_dst.mkdir(parents=True, exist_ok=True)

    # Top-level KLEE logs
    for f in logs_src.glob("klee_*.log"):
        shutil.copy2(f, logs_dst / f.name)

    # Per-run crash test cases and error files
    for run_dir in sorted(logs_src.glob("klee_run_*")):
        if not run_dir.is_dir():
            continue
        run_dst = logs_dst / run_dir.name
        has_files = False
        for f in run_dir.iterdir():
            if not f.is_file():
                continue
            if f.name.startswith("test") and f.suffix in (".ktest", ".err"):
                # Match test*.ktest and test*.XXX.err (e.g. test000001.ptr.err)
                if not has_files:
                    run_dst.mkdir(parents=True, exist_ok=True)
                    has_files = True
                shutil.copy2(f, run_dst / f.name)


def collect_spec(spec_src: Path, spec_dst: Path):
    """Collect all relevant artifacts for a single TP spec."""
    spec_dst.mkdir(parents=True, exist_ok=True)
    copy_top_level_jsons(spec_src, spec_dst)
    copy_harness(spec_src, spec_dst)
    copy_asan_replay(spec_src, spec_dst)
    copy_logs(spec_src, spec_dst)


def _file_category(bug_file: str, tool_files_set: set[str]) -> str:
    """Classify a bug file as 'tool', 'test', 'examples', 'fuzz', or 'library'."""
    basename = os.path.basename(bug_file)
    if basename in tool_files_set:
        return "tool"
    parts = bug_file.replace("\\", "/").split("/")
    for p in parts:
        if p in ("tools", "test", "tests", "examples", "fuzz"):
            return p if p != "tools" else "tool"
    return "library"


def main():
    parser = argparse.ArgumentParser(
        description="Collect SE_DETECTED artifacts from a SAILOR run"
    )
    parser.add_argument(
        "--results-dir", required=True,
        help="SE results directory (e.g. se_runs/sailor_engine/libxml2_55980_vul)"
    )
    parser.add_argument(
        "--output-dir", required=True,
        help="TP artifact destination (e.g. artifacts/libxml2_55980_vul/se_tp)"
    )
    parser.add_argument(
        "--tool-files", default=None,
        help="Comma-separated tool/test file basenames (e.g., 'tiffcrop.c,xmllint.c')"
    )
    args = parser.parse_args()

    results_dir = Path(args.results_dir)
    output_dir = Path(args.output_dir)

    # Build tool-file set from CLI arg or TOOL_FILES env var
    tool_files_set: set[str] = set()
    tool_files_str = args.tool_files or os.environ.get("TOOL_FILES", "")
    if tool_files_str:
        tool_files_set = {f.strip() for f in tool_files_str.split(",")} - {""}

    summary_path = results_dir / "summary.tsv"
    if not summary_path.exists():
        print(f"[WARN] No summary.tsv in {results_dir}, skipping TP collection")
        return

    # Read summary TSV
    with open(summary_path, "r") as f:
        reader = csv.DictReader(f, delimiter="\t")
        all_rows = list(reader)
        fieldnames = reader.fieldnames

    if not fieldnames:
        print("[WARN] summary.tsv has no header, skipping")
        return

    # Filter TP rows
    tp_rows = [r for r in all_rows if r.get("FinalStatus") == "SE_DETECTED"]

    if not tp_rows:
        print("[i] No SE_DETECTED specs found, skipping TP artifact collection")
        return

    # --- Filter out tool/test file specs ---
    if tool_files_set:
        before = len(tp_rows)
        tp_rows = [
            r for r in tp_rows
            if _file_category(r.get("BugFile", ""), tool_files_set) == "library"
        ]
        filtered = before - len(tp_rows)
        if filtered:
            print(f"[i] Filtered {filtered} tool/test-file specs from SE output")
        if not tp_rows:
            print("[i] No library SE_DETECTED specs remain after filtering")
            return

    # --- Deduplicate multi-cycle rows (same Spec name → keep last occurrence) ---
    seen_specs: dict[str, dict] = {}
    for row in tp_rows:
        seen_specs[row["Spec"]] = row  # last occurrence wins
    tp_rows_deduped = list(seen_specs.values())

    # Unique spec names
    tp_specs = sorted(set(r["Spec"] for r in tp_rows_deduped))

    # --- Crash signature dedup ---
    # Cluster by (BugFile, BugLine, BugType) — exact match
    clusters: dict[tuple[str, str, str], list[dict]] = defaultdict(list)
    for row in tp_rows_deduped:
        sig = (
            row.get("BugFile", ""),
            row.get("BugLine", ""),
            row.get("BugType", ""),
        )
        clusters[sig].append(row)

    # Assign UniqueBugID and pick representative per cluster
    bug_id_counter = 0
    bug_id_map: dict[str, str] = {}       # spec_name -> BUG-NNN
    is_duplicate: dict[str, bool] = {}     # spec_name -> True if not representative

    for sig, members in sorted(clusters.items(), key=lambda x: -len(x[1])):
        bug_id_counter += 1
        bug_id = f"BUG-{bug_id_counter:03d}"

        # Pick representative: prefer exact_match, then first
        def _rep_key(r: dict) -> tuple[int,]:
            exact = 1 if r.get("CrashRelevance") == "exact_match" else 0
            return (exact,)

        members_sorted = sorted(members, key=_rep_key, reverse=True)
        representative = members_sorted[0]["Spec"]

        for m in members_sorted:
            spec = m["Spec"]
            bug_id_map[spec] = bug_id
            is_duplicate[spec] = spec != representative

    # Clean output dir
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    # Write summary_full.tsv (all rows)
    with open(output_dir / "summary_full.tsv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        writer.writerows(all_rows)

    # Write summary_tp.tsv (library TP rows only, with UniqueBugID + IsDuplicate)
    tp_fieldnames = list(fieldnames) + ["UniqueBugID", "IsDuplicate"]
    with open(output_dir / "summary_tp.tsv", "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=tp_fieldnames, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        for row in tp_rows_deduped:
            out_row = dict(row)
            spec = row["Spec"]
            out_row["UniqueBugID"] = bug_id_map.get(spec, "")
            out_row["IsDuplicate"] = "true" if is_duplicate.get(spec, False) else ""
            writer.writerow(out_row)

    # Collect artifacts per spec
    collected = 0
    for spec_name in tp_specs:
        spec_src = results_dir / spec_name
        if not spec_src.is_dir():
            print(f"[WARN] Spec directory not found: {spec_src}")
            continue
        spec_dst = output_dir / spec_name
        collect_spec(spec_src, spec_dst)
        collected += 1

    unique_bugs = bug_id_counter
    dup_count = sum(1 for v in is_duplicate.values() if v)
    print(f"[OK] TP artifacts collected: {collected} specs -> {output_dir}")
    print(f"     Crash dedup: {len(tp_rows_deduped)} SE_DETECTED specs -> {unique_bugs} unique bugs ({dup_count} duplicates)")
    print(f"     TP summary:  {output_dir}/summary_tp.tsv ({len(tp_rows_deduped)} rows)")
    print(f"     Full summary: {output_dir}/summary_full.tsv ({len(all_rows)} rows)")


if __name__ == "__main__":
    main()
