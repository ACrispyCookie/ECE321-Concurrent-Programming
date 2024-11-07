#!/bin/bash
for i in {1..4}; do
  target_folder="assignment$i/target"

  if [ -d "$target_folder" ]; then
    rm -rf "$target_folder"/*
    echo "Emptied $target_folder"
  else
    echo "$target_folder does not exist"
  fi
done
