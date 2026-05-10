#!/bin/bash
echo "Compiling for Linux/MacOS..."
g++ 056_t1ar_prak5.cpp database.cpp -o main
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi
echo "Compilation successful. Run with: ./main"