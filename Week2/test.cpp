#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BUFF_SIZE 2048

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Error creating socket: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Define server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Unable to connect to server: %ld\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to server.\n");

    // Input loop
    char inputBuffer[BUFF_SIZE];
    while (1) {
        printf("Enter message (or type 'exit' to quit): ");
        fgets(inputBuffer, BUFF_SIZE, stdin);

        // Check if user wants to exit
        if (strcmp(inputBuffer, "exit\n") == 0) {
            break;
        }

        // Send data to server
        if (send(clientSocket, inputBuffer, strlen(inputBuffer), 0) == SOCKET_ERROR) {
            printf("Send failed: %ld\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        // Receive response from server
        char recvBuff[BUFF_SIZE];
        int recvSize = recv(clientSocket, recvBuff, BUFF_SIZE, 0);
        if (recvSize > 0) {
            recvBuff[recvSize] = '\0'; // Null-terminate the received data
            printf("Received response from server: %s\n", recvBuff);
        } else if (recvSize == 0) {
            printf("Connection closed by server.\n");
            break;
        } else {
            printf("Recv failed: %ld\n", WSAGetLastError());
            break;
        }
    }

    // Close socket and cleanup
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}