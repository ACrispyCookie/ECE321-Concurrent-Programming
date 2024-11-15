#!/bin/bash
TEST_WITH="./target/test_with_sem"
TEST_WITHOUT="./target/test_without_sem"

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
            make test_without
            echo "Running without using semaphores"
            $TEST_WITHOUT 100000
            ;;
        s)
            make test_with
            echo "Running using semaphores"
            $TEST_WITH 10000
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done
