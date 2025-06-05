#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define MAX_FILES 5
#define MAX_FILENAME 256

// Structure to track file operations
typedef struct {
    char filename[MAX_FILENAME];
    int fd;
    off_t position;
    size_t size;
    int is_open;
} FileHandle;

// Global file handle array
FileHandle file_handles[MAX_FILES] = {0};

// Function to find a free file handle
int find_free_handle() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_handles[i].is_open) {
            return i;
        }
    }
    return -1;
}

// Function to open a file with error handling
int open_file(const char* filename, int flags) {
    int handle = find_free_handle();
    if (handle == -1) {
        fprintf(stderr, "Error: Maximum number of open files reached\n");
        return -1;
    }
    
    // Open the file
    int fd = open(filename, flags, 0644);
    if (fd < 0) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return -1;
    }
    
    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "Error getting file size for '%s': %s\n", filename, strerror(errno));
        close(fd);
        return -1;
    }
    
    // Initialize file handle
    strncpy(file_handles[handle].filename, filename, MAX_FILENAME - 1);
    file_handles[handle].filename[MAX_FILENAME - 1] = '\0';
    file_handles[handle].fd = fd;
    file_handles[handle].position = 0;
    file_handles[handle].size = st.st_size;
    file_handles[handle].is_open = 1;
    
    return handle;
}

// Function to close a file with error handling
int close_file(int handle) {
    if (handle < 0 || handle >= MAX_FILES || !file_handles[handle].is_open) {
        fprintf(stderr, "Error: Invalid file handle\n");
        return -1;
    }
    
    if (close(file_handles[handle].fd) < 0) {
        fprintf(stderr, "Error closing file '%s': %s\n", 
                file_handles[handle].filename, strerror(errno));
        return -1;
    }
    
    file_handles[handle].is_open = 0;
    return 0;
}

// Function to read from a file with error handling
ssize_t read_file(int handle, void* buffer, size_t size) {
    if (handle < 0 || handle >= MAX_FILES || !file_handles[handle].is_open) {
        fprintf(stderr, "Error: Invalid file handle\n");
        return -1;
    }
    
    // Check if we're trying to read past the end of the file
    if (file_handles[handle].position >= file_handles[handle].size) {
        return 0; // EOF
    }
    
    // Adjust size if we're near the end of the file
    if (file_handles[handle].position + size > file_handles[handle].size) {
        size = file_handles[handle].size - file_handles[handle].position;
    }
    
    // Read the data
    ssize_t bytes_read = read(file_handles[handle].fd, buffer, size);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading from file '%s': %s\n", 
                file_handles[handle].filename, strerror(errno));
        return -1;
    }
    
    // Update position
    file_handles[handle].position += bytes_read;
    return bytes_read;
}

// Function to write to a file with error handling
ssize_t write_file(int handle, const void* buffer, size_t size) {
    if (handle < 0 || handle >= MAX_FILES || !file_handles[handle].is_open) {
        fprintf(stderr, "Error: Invalid file handle\n");
        return -1;
    }
    
    // Write the data
    ssize_t bytes_written = write(file_handles[handle].fd, buffer, size);
    if (bytes_written < 0) {
        fprintf(stderr, "Error writing to file '%s': %s\n", 
                file_handles[handle].filename, strerror(errno));
        return -1;
    }
    
    // Update position and size
    file_handles[handle].position += bytes_written;
    if (file_handles[handle].position > file_handles[handle].size) {
        file_handles[handle].size = file_handles[handle].position;
    }
    
    return bytes_written;
}

// Function to seek in a file with error handling
off_t seek_file(int handle, off_t offset, int whence) {
    if (handle < 0 || handle >= MAX_FILES || !file_handles[handle].is_open) {
        fprintf(stderr, "Error: Invalid file handle\n");
        return -1;
    }
    
    off_t new_position = lseek(file_handles[handle].fd, offset, whence);
    if (new_position < 0) {
        fprintf(stderr, "Error seeking in file '%s': %s\n", 
                file_handles[handle].filename, strerror(errno));
        return -1;
    }
    
    file_handles[handle].position = new_position;
    return new_position;
}

// Function to copy a file with error handling
int copy_file(const char* src_filename, const char* dst_filename) {
    int src_handle = open_file(src_filename, O_RDONLY);
    if (src_handle < 0) {
        return -1;
    }
    
    int dst_handle = open_file(dst_filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (dst_handle < 0) {
        close_file(src_handle);
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    while ((bytes_read = read_file(src_handle, buffer, BUFFER_SIZE)) > 0) {
        ssize_t bytes_written = write_file(dst_handle, buffer, bytes_read);
        if (bytes_written < 0 || bytes_written != bytes_read) {
            fprintf(stderr, "Error during file copy\n");
            close_file(src_handle);
            close_file(dst_handle);
            return -1;
        }
    }
    
    if (bytes_read < 0) {
        fprintf(stderr, "Error during file copy\n");
        close_file(src_handle);
        close_file(dst_handle);
        return -1;
    }
    
    close_file(src_handle);
    close_file(dst_handle);
    return 0;
}

int main() {
    // Example usage
    const char* test_file = "test.txt";
    const char* copy_file = "test_copy.txt";
    
    // Create and write to a test file
    int handle = open_file(test_file, O_WRONLY | O_CREAT | O_TRUNC);
    if (handle < 0) {
        return 1;
    }
    
    const char* test_data = "Hello, World!\nThis is a test file.\n";
    if (write_file(handle, test_data, strlen(test_data)) < 0) {
        close_file(handle);
        return 1;
    }
    
    close_file(handle);
    
    // Read and display the file contents
    handle = open_file(test_file, O_RDONLY);
    if (handle < 0) {
        return 1;
    }
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    printf("File contents:\n");
    while ((bytes_read = read_file(handle, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    
    if (bytes_read < 0) {
        close_file(handle);
        return 1;
    }
    
    close_file(handle);
    
    // Copy the file
    if (copy_file(test_file, copy_file) < 0) {
        return 1;
    }
    
    printf("\nFile copied successfully\n");
    
    // Clean up
    unlink(test_file);
    unlink(copy_file);
    
    return 0;
} 