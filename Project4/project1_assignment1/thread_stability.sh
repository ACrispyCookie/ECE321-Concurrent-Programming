#!/bin/bash

# Check if the required arguments are provided
if [[ $# -lt 5 ]]; then
  echo "Usage: $0 <program> <input_file> <start> <end> <iterations> [step]"
  exit 1
fi

PROGRAM=$1          # Program to run
INPUT_FILE=$2       # Static input file
START=$3            # Starting value for the last argument
END=$4              # Ending value for the last argument
ITERATIONS=$5       # Number of times to run the program for each argument
STEP=${6:-10}       # Step size (default is 10)
OUTPUT_FILE="stability_results.csv"

# Initialize the output file with headers
echo "Argument,Run,ExecutionTime,Status" > "$OUTPUT_FILE"

# Loop over the range of values
for ((VALUE=START; VALUE<=END; VALUE+=STEP)); do
  echo "Testing argument value: $VALUE"

  for ((RUN=1; RUN<=ITERATIONS; RUN++)); do
    echo "  Iteration $RUN..."
    
    # Use a temporary file to capture the output
    TEMP_OUTPUT=$(mktemp)
    
    # Start timing
    START_TIME=$(date +%s.%N)

    # Run the program in the background
    $PROGRAM "$INPUT_FILE" "$VALUE" > "$TEMP_OUTPUT" 2>&1 &
    PID=$!

    LAST_SIZE=0
    OUTPUT_CHANGED=false
    STATUS="Success"
    EXECUTION_TIME="Inf"

    # Monitor the output for changes
    while kill -0 $PID 2>/dev/null; do
      sleep 0.5  # Wait 500ms
      CURRENT_SIZE=$(stat -c%s "$TEMP_OUTPUT" 2>/dev/null || echo 0)

      if [[ $CURRENT_SIZE -eq $LAST_SIZE ]]; then
        # No change in output, start the 5-second grace timer
        echo "    No output change detected. Waiting 5 more seconds..."

        sleep 5  # Grace period
        CURRENT_SIZE=$(stat -c%s "$TEMP_OUTPUT" 2>/dev/null || echo 0)

        if [[ $CURRENT_SIZE -eq $LAST_SIZE ]]; then
          # Output still hasn't changed
          STATUS="Stuck"
          echo "    Execution $RUN marked as stuck (no output for 5.5 seconds)."
          kill -9 $PID 2>/dev/null
          wait $PID 2>/dev/null
          break
        fi
      else
        # Output has changed
        LAST_SIZE=$CURRENT_SIZE
        OUTPUT_CHANGED=true
      fi
    done

    # If the process completed successfully, calculate execution time
    if [[ $STATUS != "Stuck" ]]; then
      wait $PID
      EXIT_CODE=$?
      END_TIME=$(date +%s.%N)
      EXECUTION_TIME=$(echo "$END_TIME - $START_TIME" | bc)

      if [[ $EXIT_CODE -ne 0 ]]; then
        STATUS="Failed"
        echo "    Execution $RUN failed (exit code $EXIT_CODE)."
      fi
    fi

    # Write the result to the output file
    echo "$VALUE,$RUN,$EXECUTION_TIME,$STATUS" >> "$OUTPUT_FILE"
    echo "    Execution $RUN completed with status: $STATUS."

    # Clean up temporary file
    rm -f "$TEMP_OUTPUT"
  done
done

echo "Stability results saved to $OUTPUT_FILE"
