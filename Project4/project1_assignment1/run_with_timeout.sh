#!/bin/bash

# Usage: ./run_with_timeout_and_iterations.sh <timeout_seconds> <program> <arg1> <start_value> <end_value> <step> <iterations>

# Input validation
if [ $# -ne 7 ]; then
    echo "Usage: $0 <timeout_seconds> <program> <arg1> <start_value> <end_value> <step> <iterations>"
    exit 1
fi

# Parameters
TIMEOUT=$1            # Timeout in seconds
PROGRAM=$2            # Program to run
ARG1=$3               # Static first argument
START_VALUE=$4        # Start value for iteration
END_VALUE=$5          # End value for iteration
STEP=$6               # Step size for the argument
ITERATIONS=$7         # Number of iterations for each argument value
LOG_FILE="execution_times.csv"  # Log file to store results

# Prepare log file header if it doesn't exist
if [ ! -f "$LOG_FILE" ]; then
    echo "Argument,Iteration,ExecutionTime,Status" > "$LOG_FILE"
fi

# Iterate through the range of values
for VALUE in $(seq "$START_VALUE" "$STEP" "$END_VALUE"); do
    echo "Running $PROGRAM with $ARG1 and value $VALUE for $ITERATIONS iterations"

    # Perform multiple iterations for the current value
    for ((i = 1; i <= ITERATIONS; i++)); do
        echo "Iteration $i for value $VALUE"

        # Start the timer
        START_TIME=$(date +%s.%N)

        # Run the program with timeout
        #timeout --foreground "$TIMEOUT" 
        "./$PROGRAM" "$ARG1" "$VALUE"
        EXIT_CODE=$?

        # Stop the timer
        END_TIME=$(date +%s.%N)

        # Calculate execution time
        EXECUTION_TIME=$(echo "$END_TIME - $START_TIME" | bc)

        # Check the program's status
        if [ $EXIT_CODE -eq 0 ]; then
            STATUS="Success"
        elif [ $EXIT_CODE -eq 124 ]; then
            STATUS="Timeout"
        else
            STATUS="Failed"
        fi

        # Log the result
        echo "Argument: $VALUE, Iteration: $i, ExecutionTime: $EXECUTION_TIME seconds, Status: $STATUS"
        echo "$VALUE,$i,$EXECUTION_TIME,$STATUS" >> "$LOG_FILE"
    done
done

echo "Execution finished. Results saved to $LOG_FILE"
