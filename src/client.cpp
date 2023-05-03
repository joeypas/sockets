#include <address.hpp>
#include <socket.hpp>

using namespace std;

int main() {

    sock client([](int code, string message){
        cout << "Error: " << code << " : " << message << endl;
    });

    client.onMessageReceived = [&client](char* message, int len){
        cout << "Server => " << message << endl;

        if (string(message, len) == "EXIT") {
            client.close();
            exit(0);
        }
    };

    client.onSocketClosed = [](int code){
        cout << "Connection closed: " << code << endl;
    };

    client.connect("127.0.0.1", 8888, [&client](address_v4* addr){
        cout << "Connected to: [" << addr->getAddrStr() << ":" << addr->getPort() << "]" << endl;

        char* message = "Hello World!";
        size_t len = strlen(message);
        client.sendall(message, len);

    },
    [](int code, string message){
        cout << code << " : " << message << endl;
    });

    string input;
    getline(cin, input);
    while (true) {
        client.sendall((char*)input.c_str(), strlen(input.c_str()));
        getline(cin, input);
    }

    return 0;
}
