#!/usr/bin/env bash
# Runs the whole GoogleTest suite with AddressSanitizer + UndefinedBehaviorSanitizer.
# Fails on memory leaks, out-of-bounds access, use-after-free and undefined behaviour.
#
#   bash scripts/sanitize.sh
#
# Needs Linux/WSL/Docker, g++, make and GoogleTest. If GoogleTest is not installed system-wide,
# set GTEST_PREFIX to a folder containing include/ and lib/.
set -euo pipefail
cd "$(dirname "$0")/.."

GTEST_FLAGS=""
GTEST_LIBS="-lgtest -lgtest_main -lpthread"
if [ -n "${GTEST_PREFIX:-}" ]; then
    GTEST_FLAGS="-I${GTEST_PREFIX}/include"
    GTEST_LIBS="-L${GTEST_PREFIX}/lib -lgtest -lgtest_main -lpthread"
fi

# Build in a separate folder with the team's own Makefile, so the repo root stays untouched.
DIR=build/sanitize
mkdir -p "$DIR/tests"
cp -u ./*.cpp ./*.h Makefile "$DIR"/
cp -u tests/*.cpp "$DIR/tests"/
make -C "$DIR" -j"$(nproc 2>/dev/null || echo 2)" run_tests \
    CXXFLAGS="-std=c++11 -Wall -Wextra -g -O1 -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined $GTEST_FLAGS" \
    GTEST_LIBS="$GTEST_LIBS"

ASAN_OPTIONS="detect_leaks=1" "./$DIR/run_tests"
echo "sanitizers: no problems found"
