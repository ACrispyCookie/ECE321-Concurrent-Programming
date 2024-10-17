#!/bin/bash

# Output binary file
output_file="random_integers.bin"

# Generate integers from 1 to 10, shuffle them, and store in an array
numbers=($(shuf -i 1-10))

# Open the output file for writing binary data
> "$output_file"  # Clear the file if it exists

# Write each number to the binary file as a 32-bit integer
for num in "${numbers[@]}"; do
  printf "%08x" "$num" | xxd -r -p >> "$output_file"
done

echo "Binary file with random integers created: $output_file"
