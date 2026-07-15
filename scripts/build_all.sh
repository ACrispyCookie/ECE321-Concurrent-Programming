#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$ROOT_DIR"

assignments=(
  Project1/assignment1
  Project1/assignment2
  Project1/assignment3
  Project2/assignment1
  Project2/assignment2
  Project2/assignment3
  Project2/assignment4
  Project3/assignment1
  Project3/assignment2
  Project3/assignment3
  Project3/assignment4
  Project3/project2_assignment3
  Project3/project2_assignment4
  Project4/list
  Project4/assignment1
  Project4/assignment2
  Project4/assignment3
  Project4/project1_assignment1
  Project4/project2_assignment2
  Project4/project2_assignment3
  Project4/project2_assignment4
)

build_c_assignment() {
  local dir=$1
  echo "==> Building $dir"
  mkdir -p "$dir/target"
  make -C "$dir"
}

for dir in "${assignments[@]}"; do
  build_c_assignment "$dir"
done

if command -v javac >/dev/null 2>&1; then
  echo "==> Building Project3/assignment2_java"
  mkdir -p Project3/assignment2_java/out
  javac -d Project3/assignment2_java/out Project3/assignment2_java/src/*.java
  jar --create --file Project3/assignment2_java/out/assignment2.jar \
      --main-class Main -C Project3/assignment2_java/out .
else
  echo "WARN: javac not found; skipping Project3/assignment2_java" >&2
fi

echo "All build targets completed."
