#!/usr/bin/env bash
# Build and run the persistence framework tests against the real SQF evaluator.
# Needs clang++ (or g++) and the fmt + spdlog headers/libs (Debian/Ubuntu:
# apt install clang libfmt-dev libspdlog-dev).
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
template="$here/../mission-template"
build="${BUILD_DIR:-$(mktemp -d)}"
cxx="${CXX:-clang++}"
flags=(-std=c++20 -O1 -w -I"$root/engine" -I"$root/thirdparty" -DHAS_CLOCK_GETTIME)

for src in "$root/engine/Evaluator/express.cpp" "$root/engine/Random/randomGen.cpp" \
           "$root/engine/Poseidon/Foundation/Memory/FastAlloc.cpp" "$here/stubs.cpp"; do
    "$cxx" "${flags[@]}" -c "$src" -o "$build/$(basename "$src" .cpp).o"
done
objs=("$build/express.o" "$build/randomGen.o" "$build/FastAlloc.o" "$build/stubs.o")
"$cxx" "${flags[@]}" "$here/framework_test.cpp" "${objs[@]}" -lfmt -o "$build/framework_test"
"$cxx" "${flags[@]}" "$here/sqs_check.cpp" "${objs[@]}" -lfmt -o "$build/sqs_check"

"$build/framework_test" "$template"
"$build/sqs_check" "$template" "$template"/persistence/*.sqs "$template"/init.sqs "$template"/initJIP.sqs
