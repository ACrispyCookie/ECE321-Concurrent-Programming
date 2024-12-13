#!/bin/bash

TEST="./target/test_mycoroutines"
FILE=""

usage() {
    echo "Usage: $0 [option]"
    echo "Options:"
    echo "  -t    Run test with a text file"
    echo "  -i    Run test with an image file"
    echo "  -v    Run test with a video file"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

while getopts ":tiv" opt; do
    case $opt in
        t)
            make clear_resources
            make
            echo "Running with a text file"
            FILE="./resources/bigfile.txt"
            $TEST $FILE
            ;;
        i)
            make clear_resources
            make
            echo "Running with an image file"
            FILE="./resources/img1.jpg"
            $TEST $FILE
            ;;
        v)
            make clear_resources
            make
            echo "Running with a video file"
            FILE="./resources/vid.mp4"
            $TEST $FILE
            ;;
        \?)
            echo "Invalid option: -$OPTARG"
            usage
            ;;
    esac
done

echo "Checking diff between original file and copied files..."
if (diff -q $FILE "$FILE.copy" > /dev/null) && (diff -q $FILE "$FILE.copy2" > /dev/null)
then
    echo "PASS! The copied file is the same with the original file!"
else
    echo "FAIL! The copied file is different from the original file!"
fi
