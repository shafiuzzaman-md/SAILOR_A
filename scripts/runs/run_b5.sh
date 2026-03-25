#!/bin/bash
# B5: Agentic LLM (Claude Code) — one session per project, no time limit
# Server: <server>
cd ~/sailor_runs
export NVM_DIR="$HOME/.nvm"
source "$NVM_DIR/nvm.sh"

run_b5() {
    local proj=$1 commit=$2 libs=$3 incs=$4
    local outdir="se_runs/b5/${proj}"

    if [ -d "$outdir" ] && [ -f "$outdir/summary.tsv" ]; then
        local lines=$(wc -l < "$outdir/summary.tsv")
        if [ "$lines" -gt 1 ]; then
            echo "===== Skipping ${proj} (done) ====="
            return
        fi
    fi

    echo "===== Starting B5 for ${proj} ====="
    echo "  Mode: single session, no time limit"
    mkdir -p "$outdir"

    local args="--project $proj --dataset-root dataset/${commit}/${proj} --output-dir $outdir"
    [ -n "$libs" ] && args="$args --upstream-libs $libs"
    [ -n "$incs" ] && args="$args --extra-include-dirs $incs"

    python3 baselines/b5_agentic_llm/run_b5.py $args \
        2>&1 | tee "$outdir/run.log"

    echo "===== Done: ${proj} ====="
}

# Projects with upstream .a paths for validation
run_b5 curl_2eebc58_vul 2eebc58 \
    "dataset/2eebc58/curl_2eebc58_vul/build/lib/libcurl.a" \
    "dataset/2eebc58/curl_2eebc58_vul/include,dataset/2eebc58/curl_2eebc58_vul/lib"

run_b5 libselinux_ca10fc4_vul ca10fc4 \
    "dataset/ca10fc4/libselinux_ca10fc4_vul/libsepol/libsepol_asan.a,dataset/ca10fc4/libselinux_ca10fc4_vul/libselinux/libselinux_asan.a" \
    "dataset/ca10fc4/libselinux_ca10fc4_vul/libsepol/include,dataset/ca10fc4/libselinux_ca10fc4_vul/libselinux/include,dataset/ca10fc4/libselinux_ca10fc4_vul/libsepol/cil/include"

run_b5 sqlite_0f08d958_vul 0f08d958 \
    "dataset/0f08d958/sqlite_0f08d958_vul/libsqlite3.a" \
    "dataset/0f08d958/sqlite_0f08d958_vul"

run_b5 openssl_67b5686b_vul 67b5686b \
    "dataset/67b5686b/openssl_67b5686b_vul/libcrypto.a,dataset/67b5686b/openssl_67b5686b_vul/libssl.a" \
    "dataset/67b5686b/openssl_67b5686b_vul/include"

run_b5 mupdf_21fb0a2b_vul 21fb0a2b \
    "dataset/21fb0a2b/mupdf_21fb0a2b_vul/build/debug/libmupdf.a,dataset/21fb0a2b/mupdf_21fb0a2b_vul/build/debug/libmupdf-third.a" \
    "dataset/21fb0a2b/mupdf_21fb0a2b_vul/include,dataset/21fb0a2b/mupdf_21fb0a2b_vul/source"

run_b5 ffmpeg_f46e5144_vul f46e5144 \
    "dataset/f46e5144/ffmpeg_f46e5144_vul/libavcodec/libavcodec.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libavformat/libavformat.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libavutil/libavutil.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libswresample/libswresample.a,dataset/f46e5144/ffmpeg_f46e5144_vul/libswscale/libswscale.a" \
    "dataset/f46e5144/ffmpeg_f46e5144_vul,dataset/f46e5144/ffmpeg_f46e5144_vul/libavcodec,dataset/f46e5144/ffmpeg_f46e5144_vul/libavutil"


echo "B5 complete"
