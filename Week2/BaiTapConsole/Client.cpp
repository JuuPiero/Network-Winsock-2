#include <iostream>
#include <sstream>
#include <string>
#include <winsock2.h>
#define BUFFER_SIZE 1024

// Hàm gửi yêu cầu tới server và nhận phản hồi từ server
std::string communicateWithServer(const std::string& serverIP, int serverPort, const std::string& message) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return "";
    }

    SOCKET clientSocket;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return "";
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return "";
    }

    if (send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending data to server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return "";
    }

    char buffer[BUFFER_SIZE];
    int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error in receiving data from server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return "";
    }

    buffer[bytesReceived] = '\0';
    std::string response(buffer);

    closesocket(clientSocket);
    WSACleanup();

    return response;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " ServerIP ServerPort" << std::endl;
        return 1;
    }
    std::string serverIP = argv[1];
    int serverPort = std::stoi(argv[2]);
    std::string command, username;

    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        if(command.substr(0, 4) == "POST" || command.substr(0, 4) == "BYE") {
            command += " " + username;
        }

        std::string response = communicateWithServer(serverIP, serverPort, command);
        // std::cout << response << std::endl;
        if(response == "10") {
            username = command.substr(5);
            std::cout << "10: Logged in successfully" << std::endl;
        }
        else if(response == "11") {
            std::cout << "11: Your account is locked." << std::endl;
        }
        else if(response == "13") {
            std::cout << "13: The account has been logged in on another client." << std::endl;
        }
        else if(response == "20") {
            std::cout << "20: Article posted successfully." << std::endl;
        }
        else if(response == "21") {
            std::cout << "21: You need to log in." << std::endl;
        }
        else if(response == "30") {
            std::cout << "30: Logged out successfully." << std::endl;
            username = "";
        }
        else {
            std::cout << "99: Unknown command." << std::endl;
        }
    }

    return 0;
}