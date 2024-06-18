#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFF_SIZE 1024

// Function declarations
int createSocket();
void setupServerAddress(struct sockaddr_in *address, const char *ip, int port);
int connectToServer(int sock, const struct sockaddr_in *address);
void sendDataToServer(int sock, const char *message);
void receiveDataFromServer(int sock, char *buffer, size_t bufferSize);

int main() {
    printf("Initializing client...\n");
    int sock = createSocket();

    //setting up server address
    struct sockaddr_in address;
    setupServerAddress(&address, SERVER_IP, PORT);

    //connecting to server
    printf("Attempting to connect to server at %s:%d...\n", SERVER_IP, PORT);
    connectToServer(sock, &address);
    printf("Connected to server. You can now send messages.\n");

    //my main communication loop
    while (1) {
        char message[BUFF_SIZE];
        //getting the users input  
        printf("Enter a message (or type 'exit' to quit): ");
        fgets(message, BUFF_SIZE, stdin);

        // Trim newline character
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }
        if (strcmp(message, "exit") == 0) {
            break;
        }
        //sending data to the server
        sendDataToServer(sock, message);

        //getting and printing server response
        char buffer[BUFF_SIZE];
        receiveDataFromServer(sock, buffer, sizeof(buffer));
        printf("Server says: %s\n", buffer);
    }

    //closing socket and exiting application ):
    printf("Exiting client application...\n");
    close(sock);
    return 0;

}

int createSocket() {
    //AF_INET : address format is host and port number
    //SOCK_STREAM : TCP
    //0 : default protocol
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void setupServerAddress(struct sockaddr_in *address, const char *ip, int port) {
    //setting up the address family
    address->sin_family = AF_INET;

    //setting up the port number with htons() method
    address->sin_port = htons(port);

    //converting the String representation of the IP to Binary
    //and storing it in sin_addr
    if (inet_pton(AF_INET, ip, &address->sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

}

int connectToServer(int sock, const struct sockaddr_in *address) {


    //triying to connect to server
    if (connect(sock, (struct sockaddr *)address, sizeof(*address)) == -1) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return 0;

}

void sendDataToServer(int sock, const char *message) {
    //sending a message to the server
    if (send(sock, message, strlen(message), 0) == -1) {
        perror("send");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void receiveDataFromServer(int sock, char *buffer, size_t bufferSize) {


    ssize_t bytesReceived = recv(sock, buffer, bufferSize - 1, 0);
    if (bytesReceived == -1) {
        perror("recv");
        close(sock);
        exit(EXIT_FAILURE);
    }
    buffer[bytesReceived] = '\0'; // Null-terminate the received data

}
