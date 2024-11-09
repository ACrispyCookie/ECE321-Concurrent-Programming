#!/bin/bash
TEST="./target/assignment3"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -1    Run test that creates N cars of the same team"
    echo "  -2    Run test that creates N+1 cars of the same team"
    echo "  -3    Run test that creates N+1 cars of both teams"
    echo "  -4    Run test that creates N+1 cars of both teams but in groups"
    echo "  -5    Run test that creates 1 car"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":12345" opt; do
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
        5)
            make
            echo "Running with the 5th test"
            $TEST 1 < ./tests/5.txt
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
