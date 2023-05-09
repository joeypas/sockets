#include <address.hpp>
#include <socket.hpp>

using namespace std;

int main() {

    bool file = false;

    sock client([](int code, string message){
        cout << "Error: " << code << " : " << message << endl;
    });

    client.onMessageSent = [&file](string message){
        if (message.substr(0,3) == "RET") file = true;
    };

    client.onMessageReceived = [&client, &file](string message){
        if (!file) cout << message << endl;
        else {
            vector<char> compressed(0);
            vector<char> decompressed(0);

            buffer::add_buffer_to_vector(compressed, message.data(), message.size());
            buffer::decompress_vector(compressed, decompressed);

            cout << decompressed.data() << endl;
            file = false;
        }
        

        if (message == "EXIT") {
            client.close();
            exit(0);
        }
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
