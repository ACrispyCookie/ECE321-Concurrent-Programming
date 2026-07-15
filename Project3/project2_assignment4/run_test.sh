#!/bin/bash
TEST="./target/assignment4"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test that creates less than N passengers"
    echo "  -2    Run test that creates exactly N passengers every one second"
    echo "  -3    Run test that creates more than N passengers"
    echo "  -4    Run test that creates exactly 2*N passengers"
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
            $TEST 5 < ./tests/1.txt
            ;;
        2)
            make
            echo "Running with the 2nd test"
            $TEST 5 < ./tests/2.txt
            ;;
        3)
            make
            echo "Running with the 3rd test"
            $TEST 5 < ./tests/3.txt
            ;;
        4)
            make
            echo "Running with the 4th test"
            $TEST 5 < ./tests/4.txt
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
