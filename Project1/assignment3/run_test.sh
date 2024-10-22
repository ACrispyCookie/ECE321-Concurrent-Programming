#!/bin/bash

TEST="./target/ask3"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test with 1000 integers as input"
    echo "  -2    Run test with 10000 integers as input"
    echo "  -3    Run test with 100000 integers as input"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":123" opt; do
    case $opt in
        1)
            echo "Generating file with 1000 random integers"
            echo "Running with 1000 integers as input"
            ./target/bin_creator ./tests/integers.bin 1000
            $TEST ./tests/integers.bin
            ;;
        2)
            echo "Generating file with 1000 random integers"
            echo "Running with 10000 integers as input"
            ./target/bin_creator ./tests/integers.bin 10000
            $TEST ./tests/integers.bin
            ;;
        3)
            echo "Generating file with 1000 random integers"
            echo "Running with 100000 integers as input"
            ./target/bin_creator ./tests/integers.bin 100000
            $TEST ./tests/integers.bin
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done


echo "Checking if the file is sorted..."
if (./target/is_sorted ./tests/integers.bin)
then
    echo "PASS! The final file is sorted!"
else
    echo "FAIL! The final file is not sorted!"
fi
