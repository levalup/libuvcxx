#!/usr/bin/env bash

set -eo pipefail

if [ $# -ne 1 ]; then
    echo "Usage: $0 <tag>"
    exit 1
fi

# script parameters
TAG="$1"

# working path
HOME=$(cd "$(dirname "$0")"; pwd)
PROJECT="$HOME/.."
BUILD="$PROJECT/build/libuv-$TAG"
LIBUV="$PROJECT/libuv"

# working config
TYPE=Release  #< build and test in Release

# auto-setup config
THREADS=$(nproc 2>/dev/null || sysctl -n hw.ncpu)

# test start
echo "[INFO] Testing $TAG ..."

# make tmp build dir
mkdir -p "$BUILD"

# checkout libuv version
pushd "$LIBUV"
git checkout "$TAG"
popd

# cmake project
cmake -B "$BUILD" -S "$PROJECT" -DCMAKE_BUILD_TYPE="$TYPE"

# build project
cmake --build "$BUILD" --config "$TYPE" -- -j "$THREADS"

# run ctest
pushd "$BUILD"
ctest --build-config "$TYPE"
popd
