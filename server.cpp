#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <WinSock2.h>
#include< WS2tcpip.h >

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clientSockets;
std::mutex mtx;

void ClientHandler(SOCKET clientSocket, char ser[]) {
    char buffer[4096];
    while (true) {
        ZeroMemory(buffer, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            break;
        }

        std::cout << "Message received from "<<ser<<"client: " << buffer << "\n";

        std::lock_guard<std::mutex> lock(mtx);
        for (auto& socket : clientSockets) {
            if (socket != clientSocket) {
                send(socket, buffer, bytesReceived, 0);
            }
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Client disconnected\n";
    auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
    if (it != clientSockets.end()) {
        closesocket(clientSocket);
        clientSockets.erase(it);
    }
}

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    if (WSAStartup(ver, &wsData) != 0) {
        return 1;
    }//初始化失败处理代码

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Can't create socket\n";
        return 1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.s_addr = INADDR_ANY;

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Can't bind socket\n";
        return 1;
    }

    if (listen(listening, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Can't listen\n";
        return 1;
    }

    std::cout << "Waiting for clients to connect...\n";

    while (true) {
        sockaddr_in client;
        int clientSize = sizeof(client);
        SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Can't accept client connection\n";
            continue;
        }

        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        ZeroMemory(host, NI_MAXHOST);
        ZeroMemory(service, NI_MAXSERV);

        if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
            std::cout << "Client connected on port: " << service << "\n";
        }
        else {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            std::cout << "Client connected on port: " << ntohs(client.sin_port) << "\n";
        }

        std::lock_guard<std::mutex> lock(mtx);
        clientSockets.push_back(clientSocket);

        std::thread clientThread(ClientHandler, clientSocket,service);
        clientThread.detach();
    }

    WSACleanup();
    return 0;
}