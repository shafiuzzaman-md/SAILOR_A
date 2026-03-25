#!/bin/bash
# B5 Scheduler — runs one project per rate limit window (5 hours)
# Keeps retrying until all projects are done
# Deploy: nohup bash b5_scheduler.sh > b5_scheduler.log 2>&1 &

cd ~/sailor_runs
export NVM_DIR="$HOME/.nvm"
source "$NVM_DIR/nvm.sh"

# All projects in priority order (remaining first)
PROJECTS=(
    "libtiff_f324415_vul:f324415:dataset/f324415/libtiff_f324415_vul/build_simple/libtiff.a:dataset/f324415/libtiff_f324415_vul/libtiff"
    "libpng_747dd02_vul:747dd02::dataset/747dd02/libpng_747dd02_vul,dataset/747dd02/libpng_747dd02_vul/build"
    "libxml2_e334a9d_vul:e334a9d::dataset/e334a9d/libxml2_e334a9d_vul/include,dataset/e334a9d/libxml2_e334a9d_vul"
    "binutils_b2bc71a_vul:b2bc71a::dataset/b2bc71a/binutils_b2bc71a_vul/include,dataset/b2bc71a/binutils_b2bc71a_vul/bfd"
    "ffmpeg_f46e5144_vul:f46e5144:dataset/f46e5144/ffmpeg_f46e5144_vul/libavcodec/libavcodec.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libavformat/libavformat.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libavutil/libavutil.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libswresample/libswresample.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libswscale/libswscale.a:dataset/f46e5144/ffmpeg_f46e5144_vul,dataset/f46e5144/ffmpeg_f46e5144_vul/libavcodec,dataset/f46e5144/ffmpeg_f46e5144_vul/libavutil"
    "mupdf_21fb0a2b_vul:21fb0a2b:dataset/21fb0a2b/mupdf_21fb0a2b_vul/build/debug/libmupdf.a,dataset/21fb0a2b/mupdf_21fb0a2b_vul/build/debug/libmupdf-third.a:dataset/21fb0a2b/mupdf_21fb0a2b_vul/include,dataset/21fb0a2b/mupdf_21fb0a2b_vul/source"
)

WAIT_HOURS=5
MAX_ROUNDS=20

for round in $(seq 1 $MAX_ROUNDS); do
    echo ""
    echo "=========================================="
    echo "Round $round — $(date)"
    echo "=========================================="

    # Check rate limit
    result=$(claude --print --max-turns 1 -p 'say OK' 2>&1 | head -1)
    if echo "$result" | grep -qi 'limit\|error'; then
        echo "[$(date)] Rate limited. Waiting ${WAIT_HOURS}h..."
        sleep $((WAIT_HOURS * 3600))
        continue
    fi

    # Find next project to run
    ran_something=false
    for entry in "${PROJECTS[@]}"; do
        IFS=: read -r proj commit libs incs <<< "$entry"
        outdir="se_runs/b5/${proj}"

        # Skip if already done
        if [ -d "$outdir" ] && [ -f "$outdir/summary.tsv" ]; then
            lines=$(wc -l < "$outdir/summary.tsv")
            if [ "$lines" -gt 1 ]; then
                # Check if it was rate limited (0 cost = rate limited)
                cost=$(find "$outdir" -name agent_stdout.txt -exec grep -o '"costUSD":[0-9.]*' {} \; 2>/dev/null | awk -F: '{s+=$2} END {printf "%.2f", s}')
                if [ "$cost" != "0.00" ] && [ -n "$cost" ]; then
                    continue  # genuinely done
                fi
                echo "[$(date)] $proj was rate limited last time, re-running..."
                rm -rf "$outdir"
            fi
        fi

        echo "[$(date)] Running B5 for $proj"
        mkdir -p "$outdir"

        args="--project $proj --dataset-root dataset/${commit}/${proj} --output-dir $outdir --max-targets 9999 --timeout 600 --jobs 16"
        [ -n "$libs" ] && args="$args --upstream-libs $libs"
        [ -n "$incs" ] && args="$args --extra-include-dirs $incs"

        python3 baselines/b5_agentic_llm/run_b5.py $args 2>&1 | tee "$outdir/run.log"

        echo "[$(date)] Done: $proj"
        ran_something=true

        # Check if we hit rate limit during the run
        last_cost=$(find "$outdir" -name agent_stdout.txt -exec grep -o '"costUSD":[0-9.]*' {} \; 2>/dev/null | tail -1 | cut -d: -f2)
        if [ "$last_cost" = "0" ]; then
            echo "[$(date)] Hit rate limit during $proj. Waiting ${WAIT_HOURS}h..."
            sleep $((WAIT_HOURS * 3600))
        fi

        break  # one project per round, then check rate limit again
    done

    if [ "$ran_something" = false ]; then
        echo "[$(date)] All projects complete!"
        break
    fi
done

echo ""
echo "=========================================="
echo "B5 Scheduler finished — $(date)"
echo "=========================================="
echo ""
echo "Final results:"
for entry in "${PROJECTS[@]}"; do
    IFS=: read -r proj commit libs incs <<< "$entry"
    outdir="se_runs/b5/${proj}"
    if [ -f "$outdir/summary.tsv" ]; then
        confirmed=$(grep -c 'SE_DETECTED\|CONFIRMED' "$outdir/summary.tsv" 2>/dev/null || echo 0)
        repros=$(find "$outdir" -name 'reproducer_*.c' 2>/dev/null | wc -l)
        echo "  $proj: $repros reproducers, $confirmed confirmed"
    else
        echo "  $proj: NOT RUN"
    fi
done
