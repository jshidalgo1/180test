#!/bin/bash

# Compile the program
gcc 180.c -pthread -lm
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p output

# Define matrix sizes and thread counts
matrix_sizes=(25000 30000 40000)
thread_counts=(1 2 4 8 16 32 64)

# Run the program and save time elapsed
for size in "${matrix_sizes[@]}"; do
    for threads in "${thread_counts[@]}"; do
        echo "Running with matrix size $size and $threads threads..."
        
        # Run the program and capture the output
        output=$(./a.out <<< "$size"$'\n'"$threads")

        # Extract time elapsed from output
        time_elapsed=$(echo "$output" | grep "Time Elapsed" | awk '{print $3}')
        
        # Save results to a text file
        echo "$size,$threads,$time_elapsed" >> output/timings3.txt
    done
done

echo "All runs completed. Results saved in output/timings3.txt."