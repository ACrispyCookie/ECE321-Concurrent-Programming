#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT_DIR"

# Build outputs
find . -type d -name target -prune -exec rm -rf {} +
rm -rf Project3/assignment2_java/out Project3/assignment2_java/build

# Runtime/generated test outputs
find . -type f \( \
  -name '*.o' -o -name '*.a' -o -name '*.class' -o -name '*.jar' -o \
  -name '*.copy' -o -name '*.copy2' -o -name '*.copy.copy' -o -name '*.copy.copy2' \
\) -delete
rm -f Project1/assignment3/tests/integers.bin
rm -f results.csv stability_results.csv execution_times.csv
find Project4/project1_assignment1 -maxdepth 1 -type f \( -name 'results.csv' -o -name 'stability_results.csv' -o -name 'execution_times.csv' \) -delete

echo "Cleaned generated build and validation-check outputs."
