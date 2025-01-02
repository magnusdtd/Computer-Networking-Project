#include "./ClientSocket.hpp"

int main() {
    SetConsoleOutputCP(CP_UTF8);

    ClientSocket client(
        "./scripts/oauth2.json", 
        "./scripts/token.json", 
        "./scripts/script.ps1"
    );

    try {
        while (true) {
            std::vector<std::pair<std::string, std::string>> servers = client.discoverServers();

            std::string chosenServer = client.chooseServer(servers);

            // Connect to the chosen server
            client.connectToServer(chosenServer);

            while (!client.getStopClient()) {
                std::cout << "\t -> [Client] Client is querying for command ...\n";
                client.query("is:unread", "");
                Sleep(3000); // Sleep for 3 seconds before checking again
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred.\n";
        return 1;
    }

    std::cout << "Client has been stopped.\n";

    return 0;
}