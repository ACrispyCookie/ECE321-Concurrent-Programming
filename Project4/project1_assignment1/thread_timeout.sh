#!/bin/bash

# Check if the required arguments are provided
if [[ $# -lt 3 ]]; then
  echo "Usage: $0 <program> <input_file> <start> <end> [step]"
  exit 1
fi

PROGRAM=$1          # Program to run
INPUT_FILE=$2       # Static input file (logo.png)
START=$3            # Starting value for the last argument
END=$4              # Ending value for the last argument
STEP=${5:-10}       # Step size (default is 10)
OUTPUT_FILE="results.csv"

# Initialize the output file with headers
echo "Argument,ExecutionTime,Status" > "$OUTPUT_FILE"

# Loop over the range of values
for ((VALUE=START; VALUE<=END; VALUE+=STEP)); do
  echo "Running $PROGRAM $INPUT_FILE $VALUE..."
  
  # Use a temporary file to capture the output
  TEMP_OUTPUT=$(mktemp)
  
  # Start timing
  START_TIME=$(date +%s.%N)

  # Run the program in the background and capture its PID
  $PROGRAM "$INPUT_FILE" "$VALUE" > "$TEMP_OUTPUT" 2>&1 &
  PID=$!

  # Wait for the first 15 seconds or until the program writes to the output
  for i in {1..15}; do
    if [[ -s $TEMP_OUTPUT ]]; then
      break
    fi
    sleep 1
  done

  # Check if the output file is still empty after 15 seconds
  if [[ ! -s $TEMP_OUTPUT ]]; then
    echo "$VALUE,Inf,NoOutput" >> "$OUTPUT_FILE"
    echo "Execution for argument $VALUE had no output within 15 seconds and was marked as stuck."
    kill -9 $PID 2>/dev/null
    wait $PID 2>/dev/null
    continue
  fi

  # Wait for the program to finish and capture the end time
  wait $PID
  END_TIME=$(date +%s.%N)

  # Calculate elapsed time
  EXECUTION_TIME=$(echo "$END_TIME - $START_TIME" | bc)
  echo "$VALUE,$EXECUTION_TIME,Success" >> "$OUTPUT_FILE"
  echo "Execution for argument $VALUE completed in $EXECUTION_TIME seconds."

  # Clean up temporary file
  rm -f "$TEMP_OUTPUT"
done

echo "Results saved to $OUTPUT_FILE"
