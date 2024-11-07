#!/bin/bash
TEST="./target/ask2"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test with the 1st test"
    echo "  -2    Run test with the 2nd test"
    echo "  -3    Run test with the 3rd test"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":123" opt; do
    case $opt in
        1)
            echo "Running with the 1st test"
            $TEST 2 < ./tests/1.txt
            ;;
        2)
            echo "Running with the 2nd test"
            $TEST 4 < ./tests/2.txt
            ;;
        3)
            echo "Running with the 3rd test"
            $TEST 4 < ./tests/3.txt
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
