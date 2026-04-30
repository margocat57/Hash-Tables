#!/usr/bin/env bash

for ((i=0; i<1000000; i++)); do
    length=$(($RANDOM % 31 + 1))
    word=$(tr -dc 'a-z' < /dev/urandom | head -c "$length")
    echo "$word" 
done > "words.txt"
