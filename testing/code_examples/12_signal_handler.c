#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CHILDREN 5
#define MAX_EVENTS 10

// Global variables
volatile sig_atomic_t running = 1;
volatile sig_atomic_t event_count = 0;
pid_t child_pids[MAX_CHILDREN] = {0};
int event_queue[MAX_EVENTS] = {0};
int queue_head = 0;
int queue_tail = 0;

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nReceived SIGINT (signal %d)\n", sig);
    running = 0;
}

// Signal handler for SIGCHLD (child process state change)
void handle_sigchld(int sig __attribute__((unused))) {
    int status;
    pid_t pid;
    
    // Handle all terminated children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Child process %d terminated with status %d\n", pid, WEXITSTATUS(status));
        
        // Remove from child_pids array
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (child_pids[i] == pid) {
                child_pids[i] = 0;
                break;
            }
        }
    }
}

// Signal handler for SIGUSR1 (custom event)
void handle_sigusr1(int sig __attribute__((unused))) {
    // Add event to queue if not full
    if ((queue_tail + 1) % MAX_EVENTS != queue_head) {
        event_queue[queue_tail] = event_count++;
        queue_tail = (queue_tail + 1) % MAX_EVENTS;
        printf("Event %d queued\n", event_count - 1);
    } else {
        printf("Event queue full, event dropped\n");
    }
}

// Function to process events from queue
void process_events() {
    while (queue_head != queue_tail) {
        int event = event_queue[queue_head];
        printf("Processing event %d\n", event);
        queue_head = (queue_head + 1) % MAX_EVENTS;
        sleep(1); // Simulate event processing
    }
}

// Child process function
void child_process(int id) {
    printf("Child %d started (PID: %d)\n", id, getpid());
    
    // Send SIGUSR1 to parent every 2 seconds
    while (1) {
        sleep(2);
        if (kill(getppid(), SIGUSR1) < 0) {
            perror("kill failed");
            exit(1);
        }
    }
}

int main() {
    struct sigaction sa;
    
    // Initialize signal handlers
    memset(&sa, 0, sizeof(sa));
    
    // Set up SIGINT handler
    sa.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction SIGINT failed");
        return 1;
    }
    
    // Set up SIGCHLD handler
    sa.sa_handler = handle_sigchld;
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("sigaction SIGCHLD failed");
        return 1;
    }
    
    // Set up SIGUSR1 handler
    sa.sa_handler = handle_sigusr1;
    sa.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("sigaction SIGUSR1 failed");
        return 1;
    }
    
    // Create child processes
    for (int i = 0; i < MAX_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            // Child process
            child_process(i);
            return 0;
        } else {
            // Parent process
            child_pids[i] = pid;
        }
    }
    
    printf("Parent process (PID: %d) started\n", getpid());
    printf("Press Ctrl+C to exit\n");
    
    // Main event loop
    while (running) {
        // Process any pending events
        process_events();
        
        // Sleep briefly to prevent busy waiting
        usleep(100000); // 100ms
    }
    
    // Cleanup: terminate all child processes
    printf("\nTerminating child processes...\n");
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (child_pids[i] > 0) {
            if (kill(child_pids[i], SIGTERM) < 0) {
                perror("kill failed");
            }
        }
    }
    
    // Wait for all children to terminate
    while (wait(NULL) > 0);
    
    printf("Parent process exiting\n");
    return 0;
} 