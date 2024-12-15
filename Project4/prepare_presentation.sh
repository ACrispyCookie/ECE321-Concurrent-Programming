#!/bin/bash

CLEAR=$1

echo "Clearing binaries folders..."
for i in {1..3}; do
  target_folder="assignment$i/target"

  if [ -d "$target_folder" ]; then
    rm -rf "$target_folder"/*
    echo "Emptied $target_folder"
  else
    echo "$target_folder does not exist"
  fi
done
for i in {1..1}; do
  target_folder="project1_assignment$i/target"

  if [ -d "$target_folder" ]; then
    rm -rf "$target_folder"/*
    echo "Emptied $target_folder"
  else
    echo "$target_folder does not exist"
  fi
done
for i in {3..4}; do
  target_folder="project2_assignment$i/target"

  if [ -d "$target_folder" ]; then
    rm -rf "$target_folder"/*
    echo "Emptied $target_folder"
  else
    echo "$target_folder does not exist"
  fi
done

target_folder="list/target"

if [ -d "$target_folder" ]; then
  rm -rf "$target_folder"/*
  echo "Emptied $target_folder"
else
  echo "$target_folder does not exist"
fi

if [ "$CLEAR" == "-c" ]; then
  exit 1
fi

echo ""
echo "Making list library..."
cd list
make

echo ""
echo "Making coroutine library..."
cd ../assignment1
make

echo ""
echo "Making threads library..."
cd ../assignment2
make 



