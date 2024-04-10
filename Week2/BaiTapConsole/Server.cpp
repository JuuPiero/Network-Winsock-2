#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <thread>
#include <winsock2.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

std::map<std::string, int> accounts; // Lưu tài khoản và trạng thái (0: hoạt động, 1: bị khóa)
std::map<std::string, bool> loggedInUsers; // Lưu tài khoản đã đăng nhập
bool loadAccounts(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open account file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username;
        int status;
        if (iss >> username >> status) {
            accounts[username] = status;
        }
    }

    file.close();
    return true;
}

int handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Nhận yêu cầu từ client
    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Error in receiving data from client." << std::endl;
        return 1;
    }
    buffer[bytesReceived] = '\0';
    
    // Phân tích yêu cầu từ client
    std::istringstream iss(buffer);
    std::string requestType, username, article;
    iss >> requestType;

    if (requestType == "USER") {
        iss >> username;
        if (accounts.find(username) != accounts.end()) {
            int status = accounts[username];
            std::string response;
            if (status == 0 && loggedInUsers[username] != true) {
                response = "10";
                loggedInUsers[username] = true;
            } 
            else if(status == 0 && loggedInUsers[username] == true) {
                response = "13";
            }
            else if (status == 1) {
                response = "11";
            }
          
            send(clientSocket, response.c_str(), response.length(), 0);
        } else {
            send(clientSocket, "12", 2, 0);
        }
    } else if (requestType == "POST") {
        iss >> article;
        // std::cout << article << std::endl;
        iss >> username;
        if (username.empty()) {
            send(clientSocket, "21", 2, 0);
        } else if(loggedInUsers[username] == true) {
            // Xử lý việc đăng bài
            send(clientSocket, "20", 2, 0);
        }
    } else if (requestType == "BYE") {
        iss >> username;
        if(username.empty()) {
            send(clientSocket, "21", 2, 0);
        }
        else {
            loggedInUsers[username] = false;
            send(clientSocket, "30", 2, 0);
        }
    } else {
        send(clientSocket, "99", 2, 0);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PortNumber" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET listenSocket;
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket." << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. Listening on port " << port << std::endl;

    loadAccounts("account.txt"); // Load tài khoản từ file

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (true) {
        if ((clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize)) == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        std::cout << "Client connected." << std::endl;

        // Xử lý từng client trong một luồng riêng biệt
        std::thread(handleClient, clientSocket).detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}