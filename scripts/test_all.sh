#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT_DIR"

run() {
  local name=$1
  shift
  echo "==> $name"
  "$@"
}

run "Project1 assignment1 text pipeline" timeout 20 bash -lc 'cd Project1/assignment1 && ./run_test.sh -t && diff -q resources/bigfile.txt resources/bigfile.txt.copy && diff -q resources/bigfile.txt resources/bigfile.txt.copy2'
run "Project1 assignment2 worker scheduler" timeout 20 bash -lc 'cd Project1/assignment2 && ./run_test.sh -1'
run "Project1 assignment3 file sorting" timeout 20 bash -lc 'cd Project1/assignment3 && ./run_test.sh -1'

run "Project2 assignment1 semaphore protected counter" timeout 20 bash -lc 'cd Project2/assignment1 && ./run_test.sh -s'
run "Project2 assignment2 worker scheduler" timeout 20 bash -lc 'cd Project2/assignment2 && ./run_test.sh -1'
run "Project2 assignment3 bridge/cars" timeout 20 bash -lc 'cd Project2/assignment3 && ./run_test.sh -5'
run "Project2 assignment4 train/passengers" timeout 20 bash -lc 'cd Project2/assignment4 && ./run_test.sh -2'

run "Project3 assignment1 semaphore protected counter" timeout 20 bash -lc 'cd Project3/assignment1 && ./run_test.sh -s'
run "Project3 assignment2 worker scheduler" timeout 20 bash -lc 'cd Project3/assignment2 && ./run_test.sh -1'
run "Project3 assignment3 bridge/cars" timeout 20 bash -lc 'cd Project3/assignment3 && ./run_test.sh -5'
run "Project3 assignment4 train/passengers" timeout 20 bash -lc 'cd Project3/assignment4 && ./run_test.sh -2'

run "Project4 assignment1 coroutine copy pipeline" timeout 20 bash -lc 'cd Project4/assignment1 && ./run_test.sh -t && diff -q resources/bigfile.txt resources/bigfile.txt.copy && diff -q resources/bigfile.txt resources/bigfile.txt.copy2'
run "Project4 assignment3 readers/writers" timeout 20 bash -lc 'cd Project4/assignment3 && ./run_test.sh -1'
run "Project4 project1 assignment1 user-thread copy pipeline" timeout 20 bash -lc 'cd Project4/project1_assignment1 && ./run_test.sh -t && diff -q resources/bigfile.txt resources/bigfile.txt.copy && diff -q resources/bigfile.txt resources/bigfile.txt.copy2'
run "Project4 project2 assignment2 user-thread worker scheduler" timeout 20 bash -lc 'cd Project4/project2_assignment2 && ./run_test.sh -1'
run "Project4 project2 assignment3 user-thread bridge/cars" timeout 20 bash -lc 'cd Project4/project2_assignment3 && ./run_test.sh -5'
run "Project4 project2 assignment4 user-thread train/passengers" timeout 20 bash -lc 'cd Project4/project2_assignment4 && ./run_test.sh -2'

if [[ -f Project3/assignment2_java/out/assignment2.jar ]]; then
  run "Project3 assignment2 Java CLI help check" timeout 5 java -jar Project3/assignment2_java/out/assignment2.jar --help || true
fi

echo "Validation checks completed."
