#!/bin/bash
set -euo pipefail
shopt -s nullglob

PENGUIN_EXE="${PENGUIN_EXE:-./Penguin}"
EXAMPLES_DIR="${EXAMPLES_DIR:-examples}"

if [[ ! -x "$PENGUIN_EXE" ]]; then
    echo "Penguin executable not found at '$PENGUIN_EXE'"
    exit 1
fi

failures=0

for dir in "$EXAMPLES_DIR"/*; do
    [[ -d "$dir" ]] || continue

    PENG_FILE="$dir/main.peng"
    [[ -f "$PENG_FILE" ]] || continue

    inputs=("$dir"/in/*.in)
    if [[ ${#inputs[@]} -eq 0 ]]; then
        echo "No input files found for $PENG_FILE, skipping"
        continue
    fi

    for in_file in "${inputs[@]}"; do
        BASE_NAME=$(basename "$in_file" .in)
        OUT_FILE="$dir/out/$BASE_NAME.out"

        if [[ ! -f "$OUT_FILE" ]]; then
            echo "Expected output $OUT_FILE is missing"
            failures=1
            continue
        fi

        TEMP_OUT=$(mktemp)

        if ! "$PENGUIN_EXE" "$PENG_FILE" <"$in_file" >"$TEMP_OUT"; then
            echo "Execution failed for $in_file with program $PENG_FILE"
            failures=1
            rm -f "$TEMP_OUT"
            continue
        fi

        if python - "$OUT_FILE" "$TEMP_OUT" <<'PY'
import sys
from difflib import unified_diff
from pathlib import Path

expected_path, actual_path = sys.argv[1], sys.argv[2]
expected = Path(expected_path).read_text().splitlines()
actual = Path(actual_path).read_text().splitlines()

if expected == actual:
    sys.exit(0)

for line in unified_diff(expected, actual, fromfile=expected_path, tofile=actual_path, lineterm=""):
    print(line)
sys.exit(1)
PY
        then
            echo "Test $in_file for $PENG_FILE passed"
        else
            echo "Test $in_file for $PENG_FILE failed"
            failures=1
        fi

        rm -f "$TEMP_OUT"
    done
done

if [[ $failures -ne 0 ]]; then
    echo "Some tests failed"
    exit 1
fi

echo "All tests passed"
