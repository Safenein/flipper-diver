#!/usr/bin/env bash
# Builds and runs each tests/test_*.c on the host, with the warnings ufbt
# enables (gnu17 is gcc 12's default there) plus ASan and UBSan. Shared by
# the devenv script `unit-tests` and the CI.
#
# Usage: tools/unit-tests.sh [output dir]
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
out=${1:-$root/build/unit-tests}
# Furi-free sources linked into every test (ADR-0004).
pure_sources=(stratagems.c game.c)

mkdir -p "$out"
cd "$root"
status=0
for t in tests/test_*.c; do
    bin="$out/$(basename "$t" .c)"
    "${CC:-cc}" -std=gnu17 -Wall -Wextra -Werror -Wdouble-promotion -Wundef \
        -Wredundant-decls -g -fno-omit-frame-pointer \
        -fsanitize=address,undefined -fno-sanitize-recover=all \
        -I. -o "$bin" "$t" "${pure_sources[@]}"
    "$bin" || status=1
done
exit "$status"
