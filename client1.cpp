#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include< WS2tcpip.h >

#pragma comment(lib, "ws2_32.lib")

void ReceiveMessage(SOCKET sock) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0,sizeof(buffer));//buffer清零
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);//接收数据，一为句柄
        if (bytesReceived <= 0) {
            break;
        }

        std::cout << "Message received from server: " << buffer << "\n";
    }
}

int main() {
    WSADATA wsData;//结构体
    WORD ver = MAKEWORD(2, 2);//套接字库版本
    if (WSAStartup(ver, &wsData) != 0) {
        return 1;
    }//初始化套接字库

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);//套接字，一为协议族，二为类型
    if (sock == INVALID_SOCKET) {
        std::cerr << "Can't create socket\n";
        return 1;
    }

    sockaddr_in hint;//存储IPv4结构体
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &(hint.sin_addr));//将IPv4转成IPv6

    if (connect(sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        std::cerr << "Can't connect to server\n";
        return 1;
    }//连接

    std::cout << "Connected to server. Start typing messages...\n";

    std::thread receiveThread(ReceiveMessage, sock);//线程
    receiveThread.detach();//解绑，删除

    std::string userInput;
    while (true) {
        std::getline(std::cin, userInput);
        int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Can't send message\n";
            break;
        }//发送信息
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}