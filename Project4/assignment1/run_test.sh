#!/bin/bash
TEST="./target/test_mycoroutines"

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -u    Run test without using semaphores"
    echo "  -s    Run test using semaphores"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":us" opt; do
    case $opt in
        u)
            make
            echo "Running without using semaphores"
            $TEST 10000
            ;;
        s)
            make
            echo "Running using semaphores"
            $TEST 10000
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
