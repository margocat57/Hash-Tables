#!/usr/bin/env bash

test_name=$1
input_file=$2
output_file=$3

while read -r line; do
    if [[ -n "$line" ]]; then
        echo "$1;$line" >> "$output_file"
    fi
done < "$input_file"