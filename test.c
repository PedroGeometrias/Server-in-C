#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define PORT 8080
#define BUFF_SIZE 1024

int serverSocket = -1;

// Function declarations
int createServerSocket();
void setupServerAddress(struct sockaddr_in *address, int port);
int bindSocket(int serverSocket, const struct sockaddr_in *address);
int listenForConnections(int serverSocket);
int acceptConnection(int serverSocket);
void receiveData(int clientSocket, char *buffer, size_t bufferSize);
void sendData(int clientSocket, const char *message);
void handleSignal(int sig);
int handleClientCommunication(int fromClientSocket, int toClientSocket, char *buffer);

int main() {
    signal(SIGINT, handleSignal); // Register signal handler for graceful shutdown


    printf("Initializing server...\n");
    serverSocket = createServerSocket(); // Create server socket

    struct sockaddr_in serverAddress;
    setupServerAddress(&serverAddress, PORT); // Set up server address structure
    bindSocket(serverSocket, &serverAddress); // Bind socket to address
    listenForConnections(serverSocket); // Start listening for incoming connections

    printf("Server is running on port %d. Waiting for client connections...\n", PORT);

    // Accept connections from two clients
    int clientSocket1 = acceptConnection(serverSocket);
    int clientSocket2 = acceptConnection(serverSocket);

    // Main loop for handling client communication
    while (1) {
        char buffer[BUFF_SIZE];

        // Handle communication from client 1 to client 2
        if (handleClientCommunication(clientSocket1, clientSocket2, buffer) == -1) break;
        // Handle communication from client 2 to client 1
        if (handleClientCommunication(clientSocket2, clientSocket1, buffer) == -1) break;
    }

    // Close sockets and exit
    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);
    return 0;

}

// Signal handler for graceful shutdown
void handleSignal(int sig) {
    printf("Received signal %d, shutting down...\n", sig);
    if (serverSocket != -1) {
        close(serverSocket); // Close server socket
    }
    exit(0);
}

// Function to create a server socket
int createServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return serverSocket;
}

// Function to set up server address structure
void setupServerAddress(struct sockaddr_in *address, int port) {
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(port);
    address->sin_family = AF_INET;
}

// Function to bind the server socket to a specific address and port
int bindSocket(int serverSocket, const struct sockaddr_in *address) {
    if (bind(serverSocket, (struct sockaddr *)address, sizeof(*address)) == -1) {
        perror("bind");
        fprintf(stderr, "Binding failed. Retrying...\n");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Function to listen for incoming connections
int listenForConnections(int serverSocket) {
    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Function to accept a connection from a client
int acceptConnection(int serverSocket) {
    int clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == -1) {
        perror("accept");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from client\n");
    return clientSocket;
}

// Function to receive data from the client
void receiveData(int clientSocket, char *buffer, size_t bufferSize) {
    ssize_t bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0'; // Null-terminate the received data


    if (bytesReceived >= bufferSize - 1) {
        fprintf(stderr, "Received message too large for the buffer\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

}

// Function to send data to the client
void sendData(int clientSocket, const char *message) {
    if (send(clientSocket, message, strlen(message), 0) == -1) {
        perror("send");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }
}

// Function to handle communication between clients
int handleClientCommunication(int fromClientSocket, int toClientSocket, char *buffer) {
    memset(buffer, 0, BUFF_SIZE);
    receiveData(fromClientSocket, buffer, BUFF_SIZE);
    if (strlen(buffer) > 0) {
        sendData(toClientSocket, buffer);
        return 0;
    }
    return -1;
}
