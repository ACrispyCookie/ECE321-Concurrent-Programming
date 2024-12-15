#!/bin/bash

# Usage: ./script.sh <program> <N>

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <program> <N>"
  exit 1
fi

PROGRAM=$1
RUNS=$2

if ! [[ $RUNS =~ ^[0-9]+$ ]]; then
  echo "Error: N must be a positive integer."
  exit 1
fi

for ((i = 1; i <= RUNS; i++)); do
  echo "Run #$i..."
  $PROGRAM 2>&1 | tee output.log

  if grep -q "segmentation fault" output.log; then
    echo "Segmentation fault detected on run #$i. Stopping execution."
    exit 1
  fi

done

rm -f output.log

echo "Program completed $RUNS runs without a segmentation fault."
