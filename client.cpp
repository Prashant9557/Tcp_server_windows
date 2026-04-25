// ============================================================
//  TCP Client — Windows Version
//  Compile: g++ -o client client.cpp -lws2_32 -std=c++17
//  Run: client.exe
// ============================================================

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP   "127.0.0.1"
#define TCP_PORT    8080
#define BUFFER_SIZE 1024

SOCKET sock;

// Background thread — server se messages receive karta hai
DWORD WINAPI receive_messages(LPVOID) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) {
            std::cout << "\n[Server se disconnect ho gaye]\n";
            exit(0);
        }
        std::cout << buffer;
        std::cout.flush();
    }
    return 0;
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket failed\n";
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed — server.exe chal raha hai?\n";
        return 1;
    }

    std::cout << "Server se connected! /quit type karo bahar aane ke liye.\n";

    // Background mein receive thread
    CreateThread(nullptr, 0, receive_messages, nullptr, 0, nullptr);

    std::string msg;
    while (std::getline(std::cin, msg)) {
        if (msg == "/quit") break;
        msg += "\n";
        send(sock, msg.c_str(), (int)msg.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
