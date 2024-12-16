#!/bin/bash

# Check if the required arguments are provided
if [[ $# -lt 5 ]]; then
  echo "Usage: $0 <program> <input_file> <start> <end> <iterations> [step]"
  exit 1
fi

PROGRAM=$1          # Program to run
INPUT_FILE=$2       # Static input file (logo.png)
START=$3            # Starting value for the last argument
END=$4              # Ending value for the last argument
ITERATIONS=$5       # Number of times to run the program for each argument
STEP=${6:-10}       # Step size (default is 10)
OUTPUT_FILE="stability_results.csv"

# Initialize the output file with headers
echo "Argument,Run,ExecutionTime,Status" > "$OUTPUT_FILE"

# Loop over the range of values
for ((VALUE=START; VALUE<=END; VALUE+=STEP)); do
  INF_COUNT=0  # Counter for Inf executions
  
  echo "Testing argument value: $VALUE"

  for ((RUN=1; RUN<=ITERATIONS; RUN++)); do
    echo "  Iteration $RUN..."
    
    # Use a temporary file to capture the output
    TEMP_OUTPUT=$(mktemp)
    
    # Start timing
    START_TIME=$(date +%s)

    # Run the program in the background and capture its PID
    $PROGRAM "$INPUT_FILE" "$VALUE" > "$TEMP_OUTPUT" 2>&1 &
    PID=$!

    # Wait for the first 15 seconds or until the program writes to the output
    sleep 0.2

    # Check if the output file is still empty after 15 seconds
    if [[ ! -s $TEMP_OUTPUT ]]; then
      INF_COUNT=$((INF_COUNT + 1))
      echo "$VALUE,$RUN,Inf,NoOutput" >> "$OUTPUT_FILE"
      echo "    Execution $RUN had no output within 15 seconds and was marked as stuck."
      kill -9 $PID 2>/dev/null
      wait $PID 2>/dev/null
      rm -f "$TEMP_OUTPUT"
      continue
    fi

    # Wait for the program to finish and capture the end time
    wait $PID
    END_TIME=$(date +%s)

    # Calculate elapsed time
    EXECUTION_TIME=$((END_TIME - START_TIME))
    echo "$VALUE,$RUN,$EXECUTION_TIME,Success" >> "$OUTPUT_FILE"
    echo "    Execution $RUN completed in $EXECUTION_TIME seconds."

    # Clean up temporary file
    rm -f "$TEMP_OUTPUT"
  done

  # Log the stability analysis for this argument value
  echo "Value $VALUE completed $ITERATIONS iterations, with $INF_COUNT Inf executions."
done

echo "Stability results saved to $OUTPUT_FILE"
