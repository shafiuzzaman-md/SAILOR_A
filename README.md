# SAILOR: Static Analysis Informed and LLM-Orchestrated Symbolic Execution

## Overview
SAILOR is a pipeline for automated vulnerability discovery in C/C++ codebases.
It combines CodeQL static analysis, LLM-guided harness synthesis, and KLEE
symbolic execution to find memory safety bugs.

## Repository Structure

```
SAILOR/
├── sailor_engine/              # Core SAILOR pipeline
│   ├── scripts/
│   │   ├── run_agent_for_spec.py    # Main agent loop (LLM + KLEE per spec)
│   │   ├── asan_replay_batch.py     # Batch ASan replay for concrete validation
│   │   ├── asan_replay.py           # Single spec ASan replay
│   │   └── unified_validate.py      # Unified baseline validation
│   ├── concrete_verify.py           # Concrete validation pipeline
│   └── collect_tp_artifacts.py      # Artifact collection
│
├── baselines/                  # Baseline implementations
│   ├── b1_ossfuzz_se/               # B1: OSS-Fuzz + KLEE
│   ├── b2_llm_harness_se/           # B2: Single-shot LLM + KLEE
│   ├── b3_llm_detect/               # B3: Pure LLM detect
│   │   └── run_b3.py
│   ├── b4_sa_llm_detect/            # B4: SA + LLM detect
│   │   └── run_b4.py
│   ├── b5_agentic_llm/              # B5: Agentic (Claude Code)
│   │   └── run_b5.py
│   ├── a1_sailor_no_sa/             # A1: SAILOR − SA (ablation)
│   │   └── run_a1.py
│   └── common/                      # Shared utilities
│       ├── baseline_utils.py
│       └── harness_templates/       # Per-project validation harnesses
│
├── rules/                      # CodeQL rules
│   └── sailor-queries/
│       └── queries/                 # 22 custom CWE rules
│           ├── CWE-120_BufferOverflow.ql
│           ├── CWE-125_OutOfBoundsRead.ql
│           ├── CWE-416_UseAfterFree_*.ql
│           ├── CWE-787_OutOfBoundsWrite.ql
│           └── ...
│
├── scripts/                    # Run & validation scripts
│   ├── revalidate_against_lib.py    # Validate against upstream .a
│   ├── revalidate_baselines.py      # Strip-and-relink for B3/B4
│   ├── unified_validate.py          # Template-based validation
│   ├── collect_token_usage.py       # LLM token tracking
│   ├── fuzz_from_replay.py          # Fuzz reproduction
│   └── runs/                        # Experiment run scripts
│       ├── run_b5.sh
│       ├── run_b5_scheduler.sh
│       ├── run_sailor_deepseek.sh
│       ├── klee_watchdog.sh
│       └── ...
│
├── configs/                    # Per-project build configs
│   ├── libtiff_f324415_vul_config.sh
│   ├── mupdf_21fb0a2b_vul_config.sh
│   └── ...
│
└── export/                     # Results & artifacts
    ├── baseline_confirmed/
    │   ├── baseline_confirmed.csv        # Main results table (paper Table 3)
    │   ├── budget_breakdown.csv          # LLM/KLEE/turns per project
    │   ├── llm_token_usage.csv           # Token cost tracking
    │   └── b3/b4/b5_extended_results.csv # Per-baseline detailed results
    ├── verified_bugs/
    │   ├── results_summary.csv           # SAILOR per-project results (paper Table 1)
    │   └── <project>.csv                 # Per-project bug lists
    ├── verified_artifacts/               # 421 concrete crash reproducers
    ├── fuzz_reproduced_artifacts/        # 251 fuzz reproduction artifacts
    ├── baseline_artifacts/               # B1-B5, A1 raw results
    └── case_studies/                     # SQLite memdb, OpenSSL bf_readbuff
```

## Quick Start

### Prerequisites
- Docker (for SAILOR pipeline with KLEE)
- Python 3.10+
- GCC/Clang with ASan support
- OpenAI API key (GPT-5) or DeepSeek API key
- For B5: Claude Code CLI (`npm install -g @anthropic-ai/claude-code`)

### Documentation

| Document | Description |
|----------|-------------|
| [DATASET.md](DATASET.md) | Dataset setup: clone commands for all 10 projects at exact commits |
| [EXPERIMENTS.md](EXPERIMENTS.md) | Step-by-step reproduction of all experiments (SAILOR, B1--B5, A1) |

### Running SAILOR

```bash
# 1. Set up environment
cp .env.example .env
# Edit .env with your LLM API key

# 2. Prepare a project (CodeQL scan + spec generation)
docker run --rm \
    -v $(pwd)/dataset:/app/dataset \
    -v $(pwd)/sa_outputs:/app/sa_outputs \
    -v $(pwd)/specs:/app/specs \
    sailor bash -c "./sailor_prepare.sh <commit>/<project>"

# 3. Run SAILOR
docker run --rm \
    -e LLM_API_KEY="$LLM_API_KEY" \
    -e LLM_MODEL="gpt-5" \
    -e JOBS=128 \
    -v $(pwd)/dataset:/app/dataset \
    -v $(pwd)/specs:/app/specs \
    -v $(pwd)/se_runs:/app/se_runs \
    sailor bash -c "./sailor.sh <commit>/<project>"
```

### Running Baselines

```bash
# B1: Human harness + KLEE (runs inside Docker)
# See EXPERIMENTS.md for full Docker command

# B2: One-shot LLM + KLEE (runs inside Docker)
# See EXPERIMENTS.md for full Docker command

# B3: Pure LLM detect
python3 baselines/b3_llm_detect/run_b3.py \
    --project libtiff_f324415_vul \
    --dataset-root dataset/f324415/libtiff_f324415_vul \
    --output-dir se_runs/b3/libtiff_f324415_vul

# B4: SA + LLM detect
python3 baselines/b4_sa_llm_detect/run_b4.py \
    --project libtiff_f324415_vul \
    --dataset-root dataset/f324415/libtiff_f324415_vul \
    --sa-outputs-dir sa_outputs/libtiff_f324415_vul \
    --output-dir se_runs/b4/libtiff_f324415_vul

# B5: Agentic (Claude Code)
python3 baselines/b5_agentic_llm/run_b5.py \
    --project libtiff_f324415_vul \
    --dataset-root dataset/f324415/libtiff_f324415_vul \
    --output-dir se_runs/b5/libtiff_f324415_vul \
    --upstream-libs libtiff.a \
    --max-targets 9999 --timeout 600 --jobs 16
```

### Concrete Validation

SAILOR's concrete validation (Phase 3) runs automatically as part
of `sailor.sh`.  For baselines (B3--B5), a separate validation
step compiles crashing inputs against the `.a`:

```bash

# B3/B4: strip-and-relink against .a
python3 scripts/revalidate_baselines.py \
    --baseline b3 \
    --findings-dir se_runs/b3/<project>/findings \
    --upstream-libs <project>.a \
    --src-root dataset/<commit>/<project> \
    --output-dir /tmp/b3_validated/<project> \
    --jobs 16
```

## Target Projects (10)

| Project | Commit | LOC | Files |
|---------|--------|-----|-------|
| libxml2 | e334a9d | 149K | 204 |
| libtiff | f324415 | 95K | 150 |
| libpng | 747dd02 | 63K | 106 |
| binutils | b2bc71a | 1.84M | 2,662 |
| curl | 2eebc58 | 174K | 697 |
| OpenSSL | 67b5686b | 710K | 2,260 |
| FFmpeg | f46e5144 | 1.3M | 4,349 |
| SELinux | ca10fc4 | 190K | 551 |
| SQLite | 0f08d958 | 1.05M | 511 |
| mupdf | 21fb0a2b | 1.25M | 3,144 |
| **Total** | | **6.8M** | **14,634** |

## Results Summary

All baselines validated with the same strict standard: crash must occur
in the project's unmodified `.a`.

| Baseline | Unique | Confirmed | Detected | Non-overlapping |
|----------|--------|-----------|----------|-----------------|
| B1: OSS-Fuzz+KLEE | 0 | 0 | 19 | 0 |
| B2: LLM+KLEE | 0 | 0 | 86 | 0 |
| B3: LLM detect | 5 | 5 | 1,781 | 4 |
| B4: SA+LLM detect | 2 | 2 | 1,363 | 2 |
| B5: Agentic | 12 | 12 | 430 | 11 |
| A1: SAILOR−SA | 31 | 31 | 276 | 30 |
| **SAILOR-GPT5** | **379** | **421** | **1,345** | **378** |

### Paper-to-Artifact Mapping

| Paper Element | Artifact Location |
|---------------|-------------------|
| Table 1 (Benchmark results) | `export/verified_bugs/results_summary.csv` |
| Table 3 (Comparison) | `export/baseline_confirmed/baseline_confirmed.csv` |
| Table 4 (Harness quality) | Computed from `export/baseline_artifacts/b1/`, `b2/` summaries |
| Figure 2 (Comparison chart) | Same data as Table 3 |
| Per-project bug lists | `export/verified_bugs/<project>.csv` |
| Concrete reproducers | `export/verified_artifacts/<project>/` |
| Fuzz reproductions | `export/fuzz_reproduced_artifacts/<project>/` |
| Case studies | `export/case_studies/sqlite_memdb_266/`, `openssl_bf_readbuff_92/` |
| CodeQL rules (Table 1) | `rules/sailor-queries/queries/` |


## Running SAILOR on a New Target

To apply SAILOR to a project not in our benchmark:

```bash
# 1. Prepare the project
mkdir -p dataset/<commit>/<project_name>
cd dataset/<commit>/<project_name>
git clone <repo_url> . && git checkout <commit>

# 2. Create a build config
cat > configs/<project_name>_config.sh << 'EOF'
#!/bin/bash
export EXTRA_CFLAGS="-I${SRC_ROOT}/include"
export TOOL_FILES="test_*.c,example_*.c"
export NON_LIBRARY_FILES=""
EOF

# 3. Run Phase 1 (CodeQL scan + spec generation)
docker run --rm \
    -v $(pwd)/dataset:/app/dataset \
    -v $(pwd)/sa_outputs:/app/sa_outputs \
    -v $(pwd)/specs:/app/specs \
    -v $(pwd)/rules:/app/rules \
    sailor bash -c "./sailor_prepare.sh <commit>/<project_name>"

# 4. Run Phase 2+3 (LLM harness synthesis + KLEE + validation)
docker run --rm \
    -e LLM_API_KEY="$LLM_API_KEY" \
    -e LLM_MODEL="gpt-5" \
    -e JOBS=64 \
    -v $(pwd)/dataset:/app/dataset \
    -v $(pwd)/specs:/app/specs \
    -v $(pwd)/se_runs:/app/se_runs \
    -v $(pwd)/configs:/app/configs \
    sailor bash -c "./sailor.sh <commit>/<project_name>"

# 5. Results
cat se_runs/sailor_engine/<project_name>/summary.tsv
```

Key requirements for a new target:
- C/C++ project with a build system (autotools, cmake, or custom)
- Produces a static archive (`.a`) for validation
- Build config specifying include paths and files to exclude

