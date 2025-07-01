#!/bin/bash

# Check if the input file is provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <source_file.c>"
    exit 1
fi

# Get the filename without the extension
filename=$(basename "$1" .c)

# Compile the MPI program
mpicc -o "$filename" "$1"
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

# Run the MPI program with 4 processes
mpirun -np 4 ./"$filename"

