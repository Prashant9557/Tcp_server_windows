// ============================================================
//  Multi-Client TCP/UDP Server — WINDOWS VERSION
//  C++17 | Winsock2 | Windows Threads | CRITICAL_SECTION
//
//  Concepts covered:
//    - Socket programming  (WSAStartup, socket, bind, listen, accept)
//    - Multi-threading     (CreateThread per client)
//    - Synchronization     (CRITICAL_SECTION — Windows mutex)
//    - Memory management   (heap alloc/free per thread)
//
//  Compile: g++ -o server server.cpp -lws2_32 -std=c++17
// ============================================================

// Winsock2 — Windows ka socket library
// Linux mein <sys/socket.h> hota hai
// Windows mein <winsock2.h> hota hai
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <map>
#include <cstring>

// Winsock library link karo
#pragma comment(lib, "ws2_32.lib")

// ─── Config ──────────────────────────────────────────────────
#define TCP_PORT 8080
#define UDP_PORT 8081
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// ─── Shared State ─────────────────────────────────────────────
// Ye map saari threads share karti hain
// CRITICAL_SECTION = Windows ka mutex
std::map<SOCKET, std::string> clients;
CRITICAL_SECTION clients_lock; // mutex equivalent
int client_count = 0;

// ─── Broadcast ────────────────────────────────────────────────
void broadcast(const std::string &msg, SOCKET sender)
{
    EnterCriticalSection(&clients_lock); // mutex lock
    for (auto &[sock, name] : clients)
        if (sock != sender)
            send(sock, msg.c_str(), (int)msg.size(), 0);
    LeaveCriticalSection(&clients_lock); // mutex unlock
}

// ─── Per-Client Thread ────────────────────────────────────────
// Har client ke liye Windows ek alag thread is function pe chalaata hai
DWORD WINAPI handle_client(LPVOID arg)
{
    // heap se socket lo, free karo
    SOCKET client_sock = *static_cast<SOCKET *>(arg);
    delete static_cast<SOCKET *>(arg);

    char buffer[BUFFER_SIZE];

    // Naam maango
    send(client_sock, "Enter your name: ", 17, 0);
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes <= 0)
    {
        closesocket(client_sock);
        return 0;
    }

    std::string name(buffer, bytes);
    while (!name.empty() && (name.back() == '\n' || name.back() == '\r'))
        name.pop_back();

    // Shared map mein add karo
    EnterCriticalSection(&clients_lock);
    clients[client_sock] = name;
    client_count++;
    LeaveCriticalSection(&clients_lock);

    std::cout << "[+] " << name << " joined | Total: " << client_count << "\n";
    broadcast(">> " + name + " joined the chat!\n", client_sock);

    // Message loop
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0)
            break;

        std::string msg(buffer, n);
        while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r'))
            msg.pop_back();

        std::cout << "[" << name << "]: " << msg << "\n";
        broadcast("[" + name + "]: " + msg + "\n", client_sock);
    }

    // Cleanup
    EnterCriticalSection(&clients_lock);
    clients.erase(client_sock);
    client_count--;
    LeaveCriticalSection(&clients_lock);

    std::cout << "[-] " << name << " left | Total: " << client_count << "\n";
    broadcast(">> " + name + " left.\n", client_sock);
    closesocket(client_sock);
    return 0;
}

// ─── UDP Thread ───────────────────────────────────────────────
DWORD WINAPI udp_listener(LPVOID arg)
{
    SOCKET udp_sock = *static_cast<SOCKET *>(arg);
    delete static_cast<SOCKET *>(arg);

    char buffer[BUFFER_SIZE];
    sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);

    std::cout << "[UDP] Listening on port " << UDP_PORT << "\n";

    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(udp_sock, buffer, BUFFER_SIZE - 1, 0,
                         (sockaddr *)&client_addr, &addr_len);
        if (n <= 0)
            continue;

        std::string msg(buffer, n);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "[UDP " << ip << "]: " << msg << "\n";

        std::string reply = "[UDP Echo]: " + msg;
        sendto(udp_sock, reply.c_str(), (int)reply.size(), 0,
               (sockaddr *)&client_addr, addr_len);
    }
    return 0;
}

// ─── Main ─────────────────────────────────────────────────────
int main()
{
    // Step 1: Winsock initialize karo — Windows pe zaruri hai
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Step 2: CRITICAL_SECTION initialize karo (mutex)
    InitializeCriticalSection(&clients_lock);

    // ── TCP Setup ──────────────────────────────────────────
    SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock == INVALID_SOCKET)
    {
        std::cerr << "TCP socket failed\n";
        return 1;
    }

    // SO_REUSEADDR — restart pe "port in use" error na aaye
    int opt = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR,
               (char *)&opt, sizeof(opt));

    sockaddr_in tcp_addr{};
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(TCP_PORT);

    if (bind(tcp_sock, (sockaddr *)&tcp_addr, sizeof(tcp_addr)) == SOCKET_ERROR)
    {
        std::cerr << "TCP bind failed\n";
        return 1;
    }
    if (listen(tcp_sock, MAX_CLIENTS) == SOCKET_ERROR)
    {
        std::cerr << "TCP listen failed\n";
        return 1;
    }

    // ── UDP Setup ──────────────────────────────────────────
    SOCKET udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock == INVALID_SOCKET)
    {
        std::cerr << "UDP socket failed\n";
        return 1;
    }

    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_sock, (sockaddr *)&udp_addr, sizeof(udp_addr)) == SOCKET_ERROR)
    {
        std::cerr << "UDP bind failed\n";
        return 1;
    }

    // UDP thread start karo
    CreateThread(nullptr, 0, udp_listener, new SOCKET(udp_sock), 0, nullptr);
    Sleep(100); // UDP thread ko pehle print karne do

    std::cout << "=========================================\n";
    std::cout << "  Multi-Client TCP/UDP Chat Server\n";
    std::cout << "  (Windows Version)\n";
    std::cout << "=========================================\n";
    std::cout << "[TCP] Port " << TCP_PORT
              << "  |  [UDP] Port " << UDP_PORT << "\n";
    std::cout << "Waiting for connections...\n\n";

    // ── Accept Loop ────────────────────────────────────────
    while (true)
    {
        sockaddr_in client_addr{};
        int client_len = sizeof(client_addr);

        SOCKET client_sock = accept(tcp_sock,
                                    (sockaddr *)&client_addr,
                                    &client_len);
        if (client_sock == INVALID_SOCKET)
            continue;

        // Max clients check
        EnterCriticalSection(&clients_lock);
        int size = (int)clients.size();
        LeaveCriticalSection(&clients_lock);

        if (size >= MAX_CLIENTS)
        {
            std::string msg = "Server full. Try later.\n";
            send(client_sock, msg.c_str(), (int)msg.size(), 0);
            closesocket(client_sock);
            continue;
        }

        // Nayi thread banao
        CreateThread(nullptr, 0, handle_client,
                     new SOCKET(client_sock), 0, nullptr);
    }

    // Cleanup
    DeleteCriticalSection(&clients_lock);
    closesocket(tcp_sock);
    closesocket(udp_sock);
    WSACleanup();
    return 0;
}
