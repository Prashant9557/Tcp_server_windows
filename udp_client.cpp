// ============================================================
//  UDP Client — Windows Version
//  Compile: g++ -o udp_client udp_client.cpp -lws2_32 -std=c++17
//  Run: udp_client.exe
// ============================================================

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP  "127.0.0.1"
#define UDP_PORT   8081
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == INVALID_SOCKET) {
        std::cerr << "Socket failed\n";
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    std::cout << "UDP Client ready. Message type karo (/quit se exit):\n";

    char buffer[BUFFER_SIZE];
    std::string msg;

    while (std::getline(std::cin, msg)) {
        if (msg == "/quit") break;

        sendto(fd, msg.c_str(), (int)msg.size(), 0,
               (sockaddr*)&addr, sizeof(addr));

        memset(buffer, 0, BUFFER_SIZE);
        int len = sizeof(addr);
        int n = recvfrom(fd, buffer, BUFFER_SIZE - 1, 0,
                         (sockaddr*)&addr, &len);
        if (n > 0)
            std::cout << buffer << "\n";
    }

    closesocket(fd);
    WSACleanup();
    return 0;
}
