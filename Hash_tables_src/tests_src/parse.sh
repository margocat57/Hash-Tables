#!/usr/bin/env bash

./make_bin.sh words_alpha.txt words_alpha.bin

for ((i=0; i<1000000; i++)); do
    length=$(($RANDOM % 31 + 1))
    word=$(tr -dc 'a-z' < /dev/urandom | head -c "$length")
    echo "$word" 
done > "words.txt"

cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt 
cat words_alpha.txt >> words.txt

shuf words.txt -o words.txt

./make_bin.sh words.txt words.bin
