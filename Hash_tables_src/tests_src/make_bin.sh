#!/usr/bin/env bash

input_file=$1
output_file=$2

> "$output_file"

while read -r line; do
    if [[ -n "$line" ]]; then
        { printf '%s' "$line"; head -c $((32-${#line})) /dev/zero; } >> "$output_file"
    fi
done < "$input_file"