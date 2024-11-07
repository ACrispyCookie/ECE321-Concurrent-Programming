#!/bin/bash
TEST="./target/test"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test with 100000 as input"
    echo "  -2    Run test with 1000000 as input"
    echo "  -3    Run test with 10000000 as input"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":123" opt; do
    case $opt in
        1)
            echo "Running with the 1st test"
            $TEST 100000
            ;;
        2)
            echo "Running with the 2nd test"
            $TEST 1000000
            ;;
        3)
            echo "Running with the 3rd test"
            $TEST 10000000
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
