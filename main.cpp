#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>
#include <algorithm> // Added for std::find
using namespace std;
#pragma comment(lib, "ws2_32.lib")

bool Initialize()
{
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        cout << "Winsock initialization failed" << endl;
        return false;
    }
    return true;
}

void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
    cout << "Client connected" << endl;

    char buffer[4096];
    while (1) {
        int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesrecvd <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        string message(buffer, bytesrecvd);
        cout << "Message from client: " << message << endl;

        // Broadcast the message to all clients except the sender
        for (auto& client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.length(), 0);
            }
        }
    }

    auto it = find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
    closesocket(clientSocket);
}

int main() {
    if (!Initialize()) {
        return 1;
    }
    cout << "Server program" << endl;

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    int port = 12345;
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    if (inet_pton(AF_INET, "0.0.0.0", &serveraddr.sin_addr) != 1) {
        cout << "Setting address structure failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }
    cout << "Server has started listening on port: " << port << endl;

    vector<SOCKET> clients;
    while (1) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client socket" << endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }
        clients.push_back(clientSocket);
        thread t1(InteractWithClient, clientSocket, ref(clients));
        t1.detach();
    }

    // This code below is unreachable and does not make sense here.
    // I've commented it out.
    /*
    char buffer[4096];
    int bytesrecd = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesrecd > 0) {
        string message(buffer, bytesrecd);
        cout << "Message from client: " << message << endl;
    }

    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    */
    return 0;
}
