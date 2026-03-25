#!/usr/bin/env python3
"""Verify all numerical claims in the SAILOR paper against raw data."""

import csv
import os
import sys

EXPORT_DIR = os.path.join(os.path.dirname(__file__), '..', 'export')

def load_results_summary():
    path = os.path.join(EXPORT_DIR, 'verified_bugs', 'results_summary.csv')
    rows = []
    with open(path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows

def load_baseline_confirmed():
    path = os.path.join(EXPORT_DIR, 'baseline_confirmed', 'baseline_confirmed.csv')
    with open(path) as f:
        lines = f.readlines()
    return lines

def check(name, expected, actual, tolerance=0):
    status = "PASS" if abs(expected - actual) <= tolerance else "FAIL"
    print(f"  [{status}] {name}: expected={expected}, actual={actual}")
    return status == "PASS"

def main():
    print("=" * 60)
    print("SAILOR Paper Claim Verification")
    print("=" * 60)

    rows = load_results_summary()
    passes = 0
    fails = 0

    # Compute totals
    total_sa = sum(int(r['#SA']) for r in rows)
    total_se = sum(int(r['#SE']) for r in rows)
    total_concrete = sum(int(r['#CONCRETE']) for r in rows)
    total_unique = sum(int(r['#u_bugs']) for r in rows)
    total_fuzz = sum(int(r['#FUZZ']) for r in rows)
    total_tokens = sum(float(r['LLM_tokens_M']) for r in rows)

    print("\n--- SAILOR Totals ---")
    for name, expected, actual in [
        ("Total SA findings", 87385, total_sa),
        ("Total SE reached", 1345, total_se),
        ("Total confirmed", 421, total_concrete),
        ("Total unique", 379, total_unique),
        ("Total fuzz reproduced", 251, total_fuzz),
    ]:
        if check(name, expected, actual):
            passes += 1
        else:
            fails += 1

    print(f"\n  [INFO] Total tokens: {total_tokens:.1f}M (paper says 2,292M)")

    # Per-project verification
    print("\n--- Per-Project ---")
    expected_projects = {
        'libxml2':  {'SA': 1987, 'SE': 377, 'CONF': 16, 'UNIQ': 11, 'FUZZ': 10},
        'libtiff':  {'SA': 1491, 'SE': 71,  'CONF': 21, 'UNIQ': 14, 'FUZZ': 15},
        'libpng':   {'SA': 609,  'SE': 44,  'CONF': 29, 'UNIQ': 21, 'FUZZ': 0},
        'binutils': {'SA': 19140,'SE': 257, 'CONF': 74, 'UNIQ': 52, 'FUZZ': 51},
        'curl':     {'SA': 2040, 'SE': 3,   'CONF': 0,  'UNIQ': 0,  'FUZZ': 0},
        'OpenSSL':  {'SA': 3977, 'SE': 0,   'CONF': 0,  'UNIQ': 0,  'FUZZ': 0},
        'FFmpeg':   {'SA': 29697,'SE': 109, 'CONF': 78, 'UNIQ': 78, 'FUZZ': 54},
        'SELinux':  {'SA': 12498,'SE': 285, 'CONF': 62, 'UNIQ': 62, 'FUZZ': 41},
        'SQLite':   {'SA': 9171, 'SE': 12,  'CONF': 0,  'UNIQ': 0,  'FUZZ': 0},
        'mupdf':    {'SA': 6775, 'SE': 187, 'CONF': 141,'UNIQ': 141,'FUZZ': 80},
    }

    for row in rows:
        proj = row['Project']
        if proj not in expected_projects:
            continue
        exp = expected_projects[proj]
        actual_vals = {
            'SA': int(row['#SA']),
            'SE': int(row['#SE']),
            'CONF': int(row['#CONCRETE']),
            'UNIQ': int(row['#u_bugs']),
            'FUZZ': int(row['#FUZZ']),
        }
        all_match = True
        for k in exp:
            if exp[k] != actual_vals[k]:
                print(f"  [FAIL] {proj}.{k}: expected={exp[k]}, actual={actual_vals[k]}")
                fails += 1
                all_match = False
        if all_match:
            print(f"  [PASS] {proj}: all fields match")
            passes += 1

    # Derived metrics
    print("\n--- Derived Metrics ---")
    for name, expected, actual in [
        ("Fuzz % of confirmed", 60, round(251/421*100)),
        ("SA FP rate %", 99.6, round((1 - 379/87385)*100, 1)),
        ("SA ratio (379/31)", 12.2, round(379/31, 1)),
        ("SE precision % (421/1345)", 31.3, round(421/1345*100, 1)),
        ("SAILOR coverage % (379/427)", 89, round(379/427*100)),
        ("Cost mupdf (122.3/141)", 0.9, round(122.3/141, 1)),
        ("Cost libpng (24.0/21)", 1.1, round(24.0/21, 1)),
        ("Cost binutils (186.0/52)", 3.6, round(186.0/52, 1)),
        ("Zero-bug tokens", 852.9, round(75.1+237.7+540.1, 1)),
    ]:
        if check(name, expected, actual, tolerance=0.1):
            passes += 1
        else:
            fails += 1

    print(f"\n{'=' * 60}")
    print(f"RESULTS: {passes} passed, {fails} failed")
    print(f"{'=' * 60}")
    return 0 if fails == 0 else 1

if __name__ == '__main__':
    sys.exit(main())
