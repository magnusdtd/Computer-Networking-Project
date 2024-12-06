#include "./ClientSocket.hpp"

int main() {
    SetConsoleOutputCP(CP_UTF8);

    ClientSocket client(
        "./GmailAPI/oauth2.json", 
        "./GmailAPI/token.json", 
        "./GmailAPI/script-auto.ps1"
    );

    try {
        while (!client.getStopClient()) {
            std::cout << "\t -> [Client] Client is querying for command ...\n";
            client.query("is:unread", "");
            Sleep(3000); // Sleep for 3 seconds before checking again
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
        return 1;
    }

    std::cout << "Client has been stopped.\n";

    return 0;
}