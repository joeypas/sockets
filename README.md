# Sockets
Library built from ground up to help interface with sockets on UNIX systems. Non blocking multithreaded operations with a simple callback interface to add custom functionality. Based very loosly on boost asio, with help from [***Beej's Guide to Network Programming***](https://beej.us/guide/bgnet/html/split/index.html)

## Build
```
./build.sh
```
This will build the dependency, tests, and source files

***

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

**MORE IN-DEPTH EXAMPLES IN SRC**
***
## Utilities 
This library includes some utilities to ease common use cases
### Buffer
Utilites for compressing, decompressing, and adding data to buffers represented by character vectors

Example:
```cpp
#include <vector>
#include <string>
#include <buffer.hpp>

using namespace std;

int main() {

    const string message = "Hello World!";
    vector<char> uncompressed_buf(0);
    vector<char> compressed_buf(0);
    vector<char> decompressed_buf(0);

    // Add a string to the buffer
    buffer::add_string_to_vector(uncompressed_buf, message);

    cout << "Uncompressed Size: " << uncompressed_buf.size() << endl;

    // Compress the buffer and store the compressed data in a new buffer
    buffer::compress_vector(uncompressed_buf, compressed_buf);

    cout << "Compressed Size: " << compressed_buf.size() << endl;

    // Decompress the buffer
    buffer::decompress_vector(compressed_buf, decompressed_buf);

    cout << "Decompressed result: " << decompressed_buf.data() << endl;

    return 0;
}
```
*Note: Compression ratio increases with size, a short string might have a lager size compressed*

### File Tree

This utility helps perform tasks asynchonysly on all of the files in a directory(*Including files in sub-directories*).
When a File Tree is created it grabs all of the files and stores them in nodes. The parent node (directory being searched) has a list of pointers to it's children, and those children might also be directories pointing to more children.

Example: 
```cpp
#include "file_tree.hpp"
#include <iostream>

int main() {
    try {
        // Creates a File Tree that includes all files
        FileTree ft(fs::path("SOME_FILE"));
    
        // Get the path of the root node
        fs::path rootp = ft.getRootPath();

        // Function that will be perfomed on each file
        auto action = [rootp](shared_ptr<FileNode> node) {
            if (node->path == rootp) {
                fs::path p(node->path);
                cout << static_cast<string>(p.filename()) << endl;
            } else {
                FileNode* parent = node->getParent();
                string name;
                while (parent->getParent() != nullptr) {
                    name = static_cast<string>(parent->path.filename()) + "/" + name;
                    parent = parent->getParent();
                }
                cout << name << static_cast<string>(node->path.filename()) << endl;

            }
        };

        ft.fileAction(action);
    }
    catch (const std::exception &e) {
        cerr << e.what() << endl;
    } 
    return 0;
}
```

This example will print all of the files, including what folder they are located in.

**FOR MORE INFO ON UTILITIES LOOK AT HEADER FILES IN INCLUDE**