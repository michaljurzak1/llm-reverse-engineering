#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define MAX_FILES 10
#define MAX_FILENAME 256

// Structure to track file operations
typedef struct {
    char filename[MAX_FILENAME];
    int fd;
    size_t size;
    off_t position;
    int flags;
} FileHandle;

// Global file handle array
static FileHandle file_handles[MAX_FILES] = {0};
static int next_handle = 0;

// File operation flags
#define FILE_READ  0x01
#define FILE_WRITE 0x02
#define FILE_APPEND 0x04

// Function to open a file with error handling
int open_file(const char* filename, const char* mode) {
    if (next_handle >= MAX_FILES) {
        errno = EMFILE;
        return -1;
    }

    int flags = 0;
    if (strcmp(mode, "r") == 0) {
        flags = FILE_READ;
    } else if (strcmp(mode, "w") == 0) {
        flags = FILE_WRITE;
    } else if (strcmp(mode, "a") == 0) {
        flags = FILE_WRITE | FILE_APPEND;
    } else {
        errno = EINVAL;
        return -1;
    }

    int fd = open(filename, 
                 (flags & FILE_READ) ? O_RDONLY :
                 (flags & FILE_APPEND) ? O_WRONLY | O_CREAT | O_APPEND :
                 O_WRONLY | O_CREAT | O_TRUNC,
                 0644);

    if (fd == -1) {
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return -1;
    }

    file_handles[next_handle].fd = fd;
    strncpy(file_handles[next_handle].filename, filename, MAX_FILENAME - 1);
    file_handles[next_handle].filename[MAX_FILENAME - 1] = '\0';
    file_handles[next_handle].size = st.st_size;
    file_handles[next_handle].position = 0;
    file_handles[next_handle].flags = flags;

    return next_handle++;
}

// Function to close a file with error handling
int close_file(int handle) {
    if (handle < 0 || handle >= next_handle || file_handles[handle].fd == -1) {
        errno = EBADF;
        return -1;
    }

    int result = close(file_handles[handle].fd);
    if (result == 0) {
        file_handles[handle].fd = -1;
        file_handles[handle].filename[0] = '\0';
        file_handles[handle].size = 0;
        file_handles[handle].position = 0;
        file_handles[handle].flags = 0;
    }
    return result;
}

// Function to read from a file with error handling
ssize_t read_file(int handle, void* buffer, size_t size) {
    if (handle < 0 || handle >= next_handle || file_handles[handle].fd == -1) {
        errno = EBADF;
        return -1;
    }

    if (!(file_handles[handle].flags & FILE_READ)) {
        errno = EACCES;
        return -1;
    }

    if (file_handles[handle].position >= (off_t)file_handles[handle].size) {
        return 0;
    }

    ssize_t bytes_read = read(file_handles[handle].fd, buffer, size);
    if (bytes_read > 0) {
        file_handles[handle].position += bytes_read;
    }
    return bytes_read;
}

// Function to write to a file with error handling
ssize_t write_file(int handle, const void* buffer, size_t size) {
    if (handle < 0 || handle >= next_handle || file_handles[handle].fd == -1) {
        errno = EBADF;
        return -1;
    }

    if (!(file_handles[handle].flags & FILE_WRITE)) {
        errno = EACCES;
        return -1;
    }

    ssize_t bytes_written = write(file_handles[handle].fd, buffer, size);
    if (bytes_written > 0) {
        file_handles[handle].position += bytes_written;
        if (file_handles[handle].position > (off_t)file_handles[handle].size) {
            file_handles[handle].size = file_handles[handle].position;
        }
    }
    return bytes_written;
}

// Function to seek in a file with error handling
int seek_file(int handle, off_t offset, int whence) {
    if (handle < 0 || handle >= next_handle || file_handles[handle].fd == -1) {
        errno = EBADF;
        return -1;
    }

    off_t new_position = lseek(file_handles[handle].fd, offset, whence);
    if (new_position == -1) {
        return -1;
    }

    file_handles[handle].position = new_position;
    return 0;
}

// Function to copy a file with error handling
int copy_file(const char* src_filename, const char* dst_filename) {
    int src_handle = open_file(src_filename, "r");
    if (src_handle < 0) {
        return -1;
    }
    
    int dst_handle = open_file(dst_filename, "w");
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
    const char* test_file = "test.txt";
    const char* test_copy = "test_copy.txt";  // Renamed from copy_file to test_copy
    
    // Test file creation and writing
    int handle = open_file(test_file, "w");
    if (handle == -1) {
        perror("Error opening file for writing");
        return 1;
    }

    const char* test_data = "Hello, World!\n";
    ssize_t written = write_file(handle, test_data, strlen(test_data));
    if (written == -1) {
        perror("Error writing to file");
        close_file(handle);
        return 1;
    }

    const char* more_data = "This is a test file.\n";
    written = write_file(handle, more_data, strlen(more_data));
    if (written == -1) {
        perror("Error writing to file");
        close_file(handle);
        return 1;
    }

    close_file(handle);
    
    // Copy the file
    if (copy_file(test_file, test_copy) < 0) {
        printf("Failed to copy file\n");
        return 1;
    }
    
    // Read and display the file contents
    handle = open_file(test_file, "r");
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
    
    // Clean up
    unlink(test_file);
    unlink(test_copy);
    
    return 0;
} 