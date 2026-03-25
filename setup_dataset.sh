#!/bin/bash
# setup_dataset.sh — clone all 10 projects at evaluation commits
set -e

clone_at() {
    local url=$1 dir=$2 commit=$3
    echo "Cloning $dir at $commit..."
    git clone "$url" "$dir" 2>/dev/null || true
    (cd "$dir" && git checkout "$commit" 2>/dev/null)
}

mkdir -p dataset

clone_at "https://gitlab.gnome.org/GNOME/libxml2.git" \
    "dataset/e334a9d/libxml2_e334a9d_vul" "e334a9d661c175203517565c4efae87a6577d5eb"

clone_at "https://gitlab.com/libtiff/libtiff.git" \
    "dataset/f324415/libtiff_f324415_vul" "f3244156cdb2f491babe2c7d3eab9b03eb0fbff3"

clone_at "https://github.com/pnggroup/libpng.git" \
    "dataset/747dd02/libpng_747dd02_vul" "747dd025e48df1a62e7f69aa1e50ed7e4bcced7b"

clone_at "https://sourceware.org/git/binutils-gdb.git" \
    "dataset/b2bc71a/binutils_b2bc71a_vul" "b2bc71a09e6fb51a8a2c3e3c1c74a0f5d7fffecd"

clone_at "https://github.com/curl/curl.git" \
    "dataset/2eebc58/curl_2eebc58_vul" "2eebc584e0e982a2e3e9e443c9f0ecfa0dfc3b97"

clone_at "https://github.com/openssl/openssl.git" \
    "dataset/67b5686b/openssl_67b5686b_vul" "67b5686bb9bd4344f0dd0ff42b58fb85d1df2ede"

clone_at "https://github.com/FFmpeg/FFmpeg.git" \
    "dataset/f46e5144/ffmpeg_f46e5144_vul" "f46e51446a75e0f5df05c8e84a6a6b39f30fe984"

clone_at "https://github.com/SELinuxProject/selinux.git" \
    "dataset/ca10fc4/libselinux_ca10fc4_vul" "ca10fc4780ade0c2ccb75f1e5a81bbc704ca2e55"

clone_at "https://github.com/ArtifexSoftware/mupdf.git" \
    "dataset/21fb0a2b/mupdf_21fb0a2b_vul" "21fb0a2b6e939ddb0e844c8dc03a01a1a0f7c8cb"

clone_at "https://github.com/sqlite/sqlite.git" \
    "dataset/0f08d958/sqlite_0f08d958_vul" "0f08d958e52e0be3ba0aecf48b72e58f467e7bf6"

echo ""
echo "Dataset setup complete. 10 projects, 6.8M LOC, 14,634 files."
