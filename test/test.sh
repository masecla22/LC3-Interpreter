#!/bin/bash

RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
END="\e[0m"

rm *.otp
rm *.log

printf "Testing...\n"

err=0

for f in *.asm
do
  printf "Test $YELLOW${f%.asm}$END: "
  interpreter_output=$( ../target/lc3 -i "$f" -o "${f%.asm}.otp" 2>&1 )
  output=$( diff -q "${f%.asm}.otp" "${f%.asm}.test" 2>&1 )
  if [[ $output || $interpreter_output ]]; then
    err=1
    printf "${RED}Failed$END\n"
    echo "######## INTERPRETER IS COMPLAINING ########" >> ${f%.asm}.log
    echo "$interpreter_output" >> ${f%.asm}.log
    echo "######## Diff says output is not ok ########" >> ${f%.asm}.log
    echo "$output" >> ${f%.asm}.log
  else
    printf "${GREEN}Passed$END\n"
  fi
done

if [[ $err == 1 ]]; then
  printf "Check Logfiles for failed tests\n"
fi
