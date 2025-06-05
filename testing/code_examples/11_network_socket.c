#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

// Global variables for cleanup
int server_fd = -1;
int client_sockets[MAX_CLIENTS] = {-1};

// Signal handler for graceful shutdown
void handle_signal(int sig) {
    printf("\nReceived signal %d, cleaning up...\n", sig);
    
    // Close all client sockets
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            close(client_sockets[i]);
            client_sockets[i] = -1;
        }
    }
    
    // Close server socket
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }
    
    exit(0);
}

// Function to handle client connection
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    
    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = 5;  // 5 seconds timeout
    tv.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt failed");
        return;
    }
    
    while (1) {
        // Clear buffer
        memset(buffer, 0, BUFFER_SIZE);
        
        // Receive data from client
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected\n");
            } else {
                perror("recv failed");
            }
            break;
        }
        
        // Convert received data to network byte order (if needed)
        uint32_t value;
        memcpy(&value, buffer, sizeof(value));
        value = ntohl(value);
        
        // Process the received data
        printf("Received: %u\n", value);
        
        // Prepare response
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "Processed value: %u", value);
        
        // Send response back to client
        if (send(client_socket, response, strlen(response), 0) < 0) {
            perror("send failed");
            break;
        }
    }
}

int main() {
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Register signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    
    // Print client information
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
    printf("Client connected from %s:%d\n", client_ip, ntohs(address.sin_port));
    
    // Handle connection
    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE - 1);
    if (valread > 0) {
        printf("Received: %s\n", buffer);
        
        // Send response
        const char* response = "Hello from server";
        send(new_socket, response, strlen(response), 0);
    }
    
    close(new_socket);
    close(server_fd);
    return 0;
} 