#!/bin/bash

# Source and destination directories
SRC_DIR="../code_examples"
DEST_DIR="../code_examples_compiled"

# Create destination directory if it doesn't exist
mkdir -p "$DEST_DIR"

# Function to compile a single file
compile_file() {
    local file="$1"
    local filename=$(basename "$file")
    local name_without_ext="${filename%.*}"
    local extension="${filename##*.}"
    
    echo "Compiling $filename..."
    
    if [ "$extension" = "c" ]; then
        gcc -o "$DEST_DIR/$name_without_ext" "$file" -Wall -Wextra
    elif [ "$extension" = "cpp" ]; then
        g++ -o "$DEST_DIR/$name_without_ext" "$file" -Wall -Wextra
    fi
    
    if [ $? -eq 0 ]; then
        echo "Successfully compiled $filename"
    else
        echo "Failed to compile $filename"
    fi
}

# Find and compile all .c and .cpp files
find "$SRC_DIR" -maxdepth 1 -type f \( -name "*.c" -o -name "*.cpp" \) | while read -r file; do
    compile_file "$file"
done

echo "Compilation complete!" 