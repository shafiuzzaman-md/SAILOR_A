# Dataset Setup — Reproducing SAILOR Experiments

## Target Projects

Clone each project at the exact commit used in our evaluation:

```bash
mkdir -p dataset
```

### 1. libxml2 (149K LOC)
```bash
git clone https://gitlab.gnome.org/GNOME/libxml2.git dataset/e334a9d/libxml2_e334a9d_vul
cd dataset/e334a9d/libxml2_e334a9d_vul
git checkout e334a9d661c175203517565c4efae87a6577d5eb
cd ../../..
```

### 2. libtiff (95K LOC)
```bash
git clone https://gitlab.com/libtiff/libtiff.git dataset/f324415/libtiff_f324415_vul
cd dataset/f324415/libtiff_f324415_vul
git checkout f3244156cdb2f491babe2c7d3eab9b03eb0fbff3
cd ../../..
```

### 3. libpng (63K LOC)
```bash
git clone https://github.com/pnggroup/libpng.git dataset/747dd02/libpng_747dd02_vul
cd dataset/747dd02/libpng_747dd02_vul
git checkout 747dd025e48df1a62e7f69aa1e50ed7e4bcced7b
cd ../../..
```

### 4. binutils (1.84M LOC)
```bash
git clone https://sourceware.org/git/binutils-gdb.git dataset/b2bc71a/binutils_b2bc71a_vul
cd dataset/b2bc71a/binutils_b2bc71a_vul
git checkout b2bc71a09e6fb51a8a2c3e3c1c74a0f5d7fffecd
cd ../../..
```

### 5. curl (174K LOC)
```bash
git clone https://github.com/curl/curl.git dataset/2eebc58/curl_2eebc58_vul
cd dataset/2eebc58/curl_2eebc58_vul
git checkout 2eebc584e0e982a2e3e9e443c9f0ecfa0dfc3b97
cd ../../..
```

### 6. OpenSSL (710K LOC)
```bash
git clone https://github.com/openssl/openssl.git dataset/67b5686b/openssl_67b5686b_vul
cd dataset/67b5686b/openssl_67b5686b_vul
git checkout 67b5686bb9bd4344f0dd0ff42b58fb85d1df2ede
cd ../../..
```

### 7. FFmpeg (1.3M LOC)
```bash
git clone https://github.com/FFmpeg/FFmpeg.git dataset/f46e5144/ffmpeg_f46e5144_vul
cd dataset/f46e5144/ffmpeg_f46e5144_vul
git checkout f46e51446a75e0f5df05c8e84a6a6b39f30fe984
cd ../../..
```

### 8. SELinux (190K LOC)
```bash
git clone https://github.com/SELinuxProject/selinux.git dataset/ca10fc4/libselinux_ca10fc4_vul
cd dataset/ca10fc4/libselinux_ca10fc4_vul
git checkout ca10fc4780ade0c2ccb75f1e5a81bbc704ca2e55
cd ../../..
```

### 9. SQLite (1.05M LOC)
```bash
git clone https://github.com/sqlite/sqlite.git dataset/0f08d958/sqlite_0f08d958_vul
cd dataset/0f08d958/sqlite_0f08d958_vul
git checkout 0f08d958e52e0be3ba0aecf48b72e58f467e7bf6
cd ../../..
```
If the commit is unavailable on the GitHub mirror, download the
SQLite amalgamation (`sqlite3.c`, `sqlite3.h`) for check-in
`0f08d958` from https://www.sqlite.org/src and place it in
`dataset/0f08d958/sqlite_0f08d958_vul/`.

### 10. mupdf (1.25M LOC)
```bash
git clone --recursive https://github.com/ArtifexSoftware/mupdf.git dataset/21fb0a2b/mupdf_21fb0a2b_vul
cd dataset/21fb0a2b/mupdf_21fb0a2b_vul
git checkout 21fb0a2b6e939ddb0e844c8dc03a01a1a0f7c8cb
git submodule update --init --recursive
cd ../../..
```

## Quick Setup Script

```bash
#!/bin/bash
# setup_dataset.sh — clone all 10 projects at evaluation commits

set -e

clone_at() {
    local url=$1 dir=$2 commit=$3
    echo "Cloning $dir at $commit..."
    git clone "$url" "$dir" 2>/dev/null || true
    cd "$dir" && git checkout "$commit" 2>/dev/null && cd - >/dev/null
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

# SQLite — use amalgamation or fossil mirror
echo "SQLite: download amalgamation from sqlite.org for commit 0f08d958"

echo "Dataset setup complete. Total: 6.8M LOC, 14,634 files across 10 projects."
```

## Building ASan Libraries

Each project needs an ASan-instrumented `.a` for concrete validation:

```bash
# Example: libtiff
cd dataset/f324415/libtiff_f324415_vul
mkdir -p build && cd build
cmake .. -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_C_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O0"
make -j$(nproc)
# Output: build/libtiff/libtiff.a

# Example: SQLite (amalgamation)
cd dataset/0f08d958/sqlite_0f08d958_vul
gcc -fsanitize=address -fno-omit-frame-pointer -g -O0 -c sqlite3.c
ar rcs libsqlite3.a sqlite3.o

# Example: mupdf
cd dataset/21fb0a2b/mupdf_21fb0a2b_vul
make -j$(nproc) build=debug \
    XCFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O0" \
    XLIBS="-fsanitize=address" libs
# Output: build/debug/libmupdf.a, build/debug/libmupdf-third.a
```

## Docker Image

The SAILOR Docker image contains KLEE 3.1, Clang/LLVM 14, CodeQL, and all
required tools. Build from the provided Dockerfile or pull from the registry:

```bash
docker build -t sailor .
# or
docker pull <registry>/sailor:latest
```

## Verification

After setup, verify the dataset:
```bash
for d in dataset/*/; do
    proj=$(ls "$d")
    loc=$(find "$d" -name "*.c" | xargs wc -l 2>/dev/null | tail -1 | awk '{print $1}')
    echo "$proj: $loc lines"
done
```

Expected output: ~6.8M total LOC across 10 projects.
