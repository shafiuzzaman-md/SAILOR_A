# SAILOR Artifact Documentation

## Artifact Overview

This artifact accompanies the paper *"SAILOR: Static Analysis Informed LLM-Orchestrated Symbolic Execution"* submitted to ASE 2026.

### What is included

| Component | Description | Location |
|-----------|-------------|----------|
| **SAILOR pipeline** | Core LLM-orchestrated SE engine | `sailor_engine/` |
| **Baselines (B1-B5, A1)** | All 6 comparison implementations | `baselines/` |
| **CodeQL rules** | 21 custom vulnerability queries | `rules/sailor-queries/` |
| **Project configs** | Build configs for 10 target projects | `configs/` |
| **Results data** | All CSVs backing paper tables/figures | `export/` |
| **Verified artifacts** | 2,904 concrete crash reproducers | `export/verified_artifacts/` |
| **Fuzz reproductions** | 1,182 fuzz reproduction artifacts | `export/fuzz_reproduced_artifacts/` |
| **Baseline artifacts** | Raw baseline results | `export/baseline_artifacts/` |
| **Claim verification** | Automated verification of paper claims | `scripts/verify_claims.py` |

### What is NOT included
- Target project source code (cloned via `setup_dataset.sh`)
- LLM API keys (user provides their own)
- Pre-built Docker images (built from `Dockerfile`)

## Getting Started

```bash
# 1. Build Docker image
docker build -t sailor .

# 2. Set up dataset (clones 10 projects at exact commits)
bash setup_dataset.sh

# 3. Configure API keys
cp .env.example .env
# Edit .env with your OpenAI API key

# 4. Run SAILOR on a project
docker run --rm --env-file .env \
    -v $(pwd)/dataset:/app/dataset \
    -v $(pwd)/se_runs:/app/se_runs \
    sailor bash -c "./sailor.sh libtiff_f324415_vul"
```

See `EXPERIMENTS.md` for full reproduction instructions.

## Verifying Paper Claims

```bash
# Run automated verification (no API keys needed)
python3 scripts/verify_claims.py
```

This script checks all numerical claims in the paper against the raw data in `export/`. Expected output: 24/24 PASS.

See `export/CLAIM_VERIFICATION.md` for detailed verification of every claim.

## Artifact Structure

```
SAILOR/
├── Dockerfile                    # Docker build for KLEE + Clang + CodeQL
├── sailor.sh                     # Main pipeline entry point
├── .env.example                  # Environment configuration template
├── LICENSE                       # MIT License
├── README.md                     # Overview and usage
├── DATASET.md                    # Dataset setup instructions
├── EXPERIMENTS.md                # Full experiment reproduction guide
├── ARTIFACTS.md                  # This file
│
├── sailor_engine/                # Core SAILOR pipeline
│   ├── scripts/
│   │   └── run_agent_for_spec.py # LLM agent loop (4,600+ lines)
│   ├── concrete_verify.py        # Phase 3: ASan replay
│   └── run_worker.sh             # Parallel spec execution
│
├── baselines/                    # All baseline implementations
│   ├── b1_ossfuzz_se/            # Human harness + KLEE
│   ├── b2_llm_harness_se/       # Single-shot LLM + KLEE
│   ├── b3_llm_detect/           # Pure LLM detection
│   ├── b4_sa_llm_detect/        # SA + LLM triage
│   ├── b5_agentic_llm/          # Autonomous agent (Claude Code)
│   └── a1_sailor_no_sa/         # SAILOR ablation (no SA)
│
├── rules/sailor-queries/         # 21 custom CodeQL queries
│   └── queries/                  # CWE-120, 125, 416, 476, 787, ...
│
├── configs/                      # Per-project build configurations
│   └── <project>_config.sh       # 10 project configs
│
├── scripts/                      # Utility scripts
│   ├── verify_claims.py          # Paper claim verification
│   ├── revalidate_against_lib.py # Upstream .a validation
│   ├── revalidate_baselines.py   # Baseline validation
│   └── fuzz_from_replay.py       # Fuzz reproduction
│
└── export/                       # All results and evidence
    ├── CLAIM_VERIFICATION.md     # Detailed claim verification
    ├── baseline_confirmed/
    │   └── baseline_confirmed.csv # Main comparison table (Table 2)
    ├── verified_bugs/
    │   ├── results_summary.csv   # Main results table (Table 1)
    │   └── <project>.csv         # Per-project bug lists
    ├── verified_artifacts/       # Concrete reproducers (per project)
    ├── fuzz_reproduced_artifacts/ # Fuzz reproduction results
    ├── baseline_artifacts/       # Raw B1-B5/A1 results
    └── case_studies/             # Detailed case study artifacts
```

## Key Data Files

| File | Paper Reference | Content |
|------|----------------|---------|
| `export/verified_bugs/results_summary.csv` | Table 1 | Per-project SA/SE/Confirmed/Unique/Fuzz/Tokens |
| `export/baseline_confirmed/baseline_confirmed.csv` | Table 2 | Per-project, per-baseline confirmed/detected |
| `export/baseline_confirmed/budget_breakdown.csv` | RQ1 (Cost) | KLEE runs, tokens, wall-clock per project |
| `export/verified_bugs/<project>.csv` | Individual bugs | Bug ID, file, line, function, error type |

## Reproducing Specific Results

### Table 1 (SAILOR results)
```bash
# Results are pre-computed in export/verified_bugs/results_summary.csv
# To rerun SAILOR on a specific project:
docker run --rm --env-file .env \
    -v $(pwd):/app sailor bash -c \
    "./sailor.sh <project> && python3 scripts/revalidate_against_lib.py <project>"
```

### Table 2 (Baseline comparison)
```bash
# Run all baselines on a project:
bash baselines/run_all.sh <project>
# Validate results:
python3 scripts/revalidate_baselines.py --baseline <B1-B5> --project <project>
```

### Figure (Comparison chart)
The chart data is derived from Table 2. Verify with:
```bash
python3 scripts/verify_claims.py  # Checks chart coordinates match table
```

## Hardware Requirements

- **Minimum**: 16 cores, 32GB RAM (runs but slow)
- **Recommended**: 128 cores, 251GB RAM (matches paper configuration)
- **Storage**: ~50GB for dataset + SE runs per project
- **Time**: 18h (libpng) to ~1,500h (FFmpeg) per project with 128 workers
