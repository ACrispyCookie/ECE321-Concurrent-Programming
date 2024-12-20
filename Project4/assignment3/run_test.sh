#!/bin/bash
TEST="./target/assignment3"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test that creates 4 readers at the same time"
    echo "  -2    Run test that creates 1 writer and 3 readers"
    echo "  -3    Run test that creates 2 writers and 4 readers"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":1234" opt; do
    case $opt in
        1)
            make
            echo "Running with the 1st test"
            $TEST < ./tests/1.txt
            ;;
        2)
            make
            echo "Running with the 2nd test"
            $TEST < ./tests/2.txt
            ;;
        3)
            make
            echo "Running with the 3rd test"
            $TEST < ./tests/3.txt
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
