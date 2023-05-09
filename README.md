# Sockets
Library built from ground up to help interface with sockets on UNIX systems. Non blocking multithreaded operations with a simple callback interface to add custom functionality. Based very loosly on boost asio, with help from ***Beej's Guide to Network Programming***.

https://beej.us/guide/bgnet/html/split/index.html

## Build
```
./build.sh
```
This will build the dependency, tests, and source files

## Usage

Example client
```cpp
#include <address.hpp>
#include <socket.hpp>

using namespace std;

int main() {

    sock client([](int code, string message){
        //Error callback
    });

    client.onMessageReceived = [](string message){
        //Message recieved callback 
    };

    client.onSocketClosed = [](int code){
        //Socket Closed callback
    };

    client.connect("127.0.0.1", 8888, [](address_v4* addr){
        //Callback that runs when client connects
    },
    [](int code, string message){
        //Error connecting
    });

    //Loop to keep program running
    string input;
    getline(cin, input);
    while (true) {
        client.sendall(input);
        getline(cin, input);
    }

    return 0;
}
```

Example server
```cpp
#include <address.hpp>
#include <socket.hpp>
#include <acceptor.hpp>

using namespace std;

int main() {
    //Acceptor is a socket that can accept incoming connections
    acceptor server([](int code, string message){
        //Error function
    });

    //Creates a socket on a new connection
    server.onNewConnection = [](sock* s){

        //Set the callbacks on the new socket
        s->onMessageReceived = [](string message){};
        s->onSocketClosed = [](int error_code){};
    };

    //Bind the server to an IP and port, and set the error callback
    server.bind("HOST IP", HOST_PORT, [](int error_code, string message){
        //Error callback
    });

    //Begin listening for new connections, loops forever
    server.beginEventLoop([](int error_code, string message){
        //Error callback
    });

    //Loop to keep program running
    string input;
    getline(cin, input);
    while (true) {
        client.sendall(input);
        getline(cin, input);
    }

    return 0;
}
```