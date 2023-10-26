#!/bin/bash

PENGUIN_EXE="./Penguin.exe"
EXAMPLES_DIR="examples"

for dir in $EXAMPLES_DIR/*; do
    if [ -d "$dir" ]; then
        PENG_FILE="$dir/$(basename $dir).peng"

        if [ -f "$PENG_FILE" ]; then
            for in_file in $dir/in/*.in; do
                BASE_NAME=$(basename "$in_file" .in)
                OUT_FILE="$dir/out/$BASE_NAME.out"
                TEMP_OUT="$dir/out/${BASE_NAME}_temp.out"

                $PENGUIN_EXE "$PENG_FILE" < "$in_file" > "$TEMP_OUT"

                if diff "$TEMP_OUT" "$OUT_FILE" &>/dev/null; then
                    echo "Test $in_file for $PENG_FILE passed."
                else
                    echo "Test $in_file for $PENG_FILE failed."
                fi

                rm "$TEMP_OUT"
            done
        fi
    fi
done
