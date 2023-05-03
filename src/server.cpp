#include <address.hpp>
#include <socket.hpp>
#include <acceptor.hpp>
#include <chrono>

using namespace std;

int main() {
    acceptor server([](int code, std::string message){
        cout << "Server: (" << code << " : " << message << endl;
    });

    server.onNewConnection = [](sock* s) {
        auto addr = s->getAddr();
        string addrStr(addr->getAddrStr());

        cout << "New connection: " << addr->getAddrStr() << ":" << addr->getPort() << endl;

        s->onMessageReceived = [s, addrStr](char* message, size_t len) {
            cout << "[" << addrStr << "]: " << message << endl;
            string ret(message, len);
            if (ret == "DONE") {
                string r = "EXIT";
                size_t m_len = strlen(r.c_str());
                s->sendall((char*)r.c_str(), m_len);
            } else {
                ret = "Recieved: \"" + ret + "\"";
                size_t m_len = strlen(ret.c_str());
                s->sendall((char*)ret.c_str(), m_len);
            }
        };

        s->onSocketClosed = [addrStr](int code) {
            cout << "Socket closed: " << addrStr << " => " << code << endl;
            cout << flush;
        };
    };

    server.bind("127.0.0.1", 8888, [](int code, string message) {
        cout << code << " : " << message << endl;
    });

    server.beginEventLoop([](int code, string message){
        cout << code << " : " << message << endl;
    });

    string input;
    cout << "> ";
    getline(cin, input);
    while (input != "exit") {
        cout << "> ";
        getline(cin, input);
    }

    server.close();

    return 0;
}