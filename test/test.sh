#!/bin/bash

# Test script to test the accuracy of the interpreter

RED="\e[31m"
GREEN="\e[32m"
YELLOW="\e[33m"
END="\e[0m"

rm *.otp
rm *.log

printf "Testing...\n"

# Uncomment the lines in here to do memory dump comparison upon halt

err=0
# mem=0


for f in *.asm
do
  printf "Test $YELLOW${f%.asm}$END: "
  interpreter_output=$( ../target/lc3 -i "$f" -o "${f%.asm}.otp" 2>&1 )
  output=$( diff -q "${f%.asm}.otp" "${f%.asm}.test" 2>&1 )
  # memory=$( cmp "${f%.asm}.mem" "${f%.asm}.mem2")
  if [[ $output || $interpreter_output ]]; then
    err=1
    printf "${RED}Failed$END "
    echo "######## INTERPRETER IS COMPLAINING ########" >> ${f%.asm}.log
    echo "$interpreter_output" >> ${f%.asm}.log
    echo "######## Diff says output is not ok ########" >> ${f%.asm}.log
    echo "$output" >> ${f%.asm}.log
  else
    printf "${GREEN}Passed$END "
  fi

  # if [[ $memory ]]; then
  #   mem=1
  #   printf "(${YELLOW}Warning: Memory dump is not consistent$END)"
  # fi

  printf "\n"

done

if [[ $err == 1 ]]; then
  printf "Check Logfiles for failed tests\n"
fi

# if [[ $mem == 1 ]]; then
#   printf "Check memory for files warned\n"
# fi
