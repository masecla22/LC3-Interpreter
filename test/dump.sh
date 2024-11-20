#!/bin/bash

for i in $(find . -type f -name *.asm -print)
do
  exec lc3as "$i"
  exec lc3sim "${i%.asm}.obj"
  exec mv "dump.bin" "${i%.asm}.bin"
done
