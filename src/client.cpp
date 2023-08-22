#include <address.hpp>
#include <socket.hpp>
#include <snappy.h>
#include <mutex>

using namespace std;

int main() {
    mutex m;
    bool file = false;

    sock client([](int code, string message){
        cout << "Error: " << code << " : " << message << endl;
    });

    client.onMessageSent = [&file](string_view message){
        if (message.starts_with("RET")) file = true;
    };

    client.onMessageReceived = [&client, &file, &m](string message){
        m.lock();
        if (!file) cout << message << endl;
        else {
            string decompressed;

            snappy::Uncompress(message.data(), message.size(), &decompressed);

            cout << decompressed << endl;
            file = false;
        }
        

        if (message == "EXIT") {
            client.close();
            client.~sock();
            exit(0);
        }
        m.unlock();
    };

    client.onSocketClosed = [](int code){
        cout << "Connection closed: " << code << endl;
    };

    client.connect("127.0.0.1", 8888, [&client](address_v4* addr){
        cout << "Connected to: [" << addr->getAddrStr() << ":" << addr->getPort() << "]" << endl;

        std::string message = "Hello World!";
        
        client.sendall(message);

    },
    [](int code, string message){
        cout << code << " : " << message << endl;
    });

    string input;
    getline(cin, input);
    while (true) {
        client.sendall(input);
        getline(cin, input);
    }

    return 0;
}
