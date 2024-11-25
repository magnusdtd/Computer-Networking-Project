#include "ServerSocket.hpp"

// Function to handle client connection
void handleClientConnection(ServerSocket &server, SOCKET &clientSocket) {
    clientSocket = accept(server.getSocket(), nullptr, nullptr);
    // Accept connection from client
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed.\n";
        return;
    }

    char buffer[1024] = {0};
    int bytesReceived = 0;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\t-> [Response from client] " << buffer << '\n';
            std::string message(buffer);
            server.handleEvent(clientSocket, message);
        } else {
            std::cerr << "\t-> [Failed to receive response from the client]\n";
            break;
        }
    }
    closesocket(clientSocket);
}

int main() {
    ServerSocket server;
    SOCKET clientSocket;

    std::thread serverThread(handleClientConnection, std::ref(server), std::ref(clientSocket));
    serverThread.join();

    return 0;
}