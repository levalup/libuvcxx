#!/usr/bin/env bash

set -eo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <ver>"
    exit 1
fi

# script parameters
STD="$1"

# working path
HOME=$(cd "$(dirname "$0")"; pwd)
PROJECT="$HOME/.."
BUILD="$PROJECT/build/std-cxx$STD"

# working config
TYPE=Release  #< build and test in Release

# auto-setup config
THREADS=$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# test start
echo "[INFO] Testing std=c++$STD ..."

# make tmp build dir
mkdir -p "$BUILD"

# cmake project
cmake -B "$BUILD" -S "$PROJECT" -DCMAKE_BUILD_TYPE="$TYPE" -DSTD="$STD"

# build project
cmake --build "$BUILD" --config "$TYPE" -- -j "$THREADS"

# run ctest
pushd "$BUILD"
ctest --build-config "$TYPE"
popd
