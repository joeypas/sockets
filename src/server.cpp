#include <address.hpp>
#include <socket.hpp>
#include <acceptor.hpp>
#include <chrono>
#include <file_tree.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

int main() {

    // Get the current directory when the program starts
    char* cwd = new char[FILENAME_MAX];
    getcwd(cwd, FILENAME_MAX);

    string workingDIR(cwd);
    delete[] cwd;

    // Create the acceptor with the on error function
    acceptor server([](int code, std::string message){
        cout << "Server: (" << code << " : " << message << endl;
    });

    // Set the callback function for when new connection is accepted passes socket that was created
    server.onNewConnection = [&workingDIR](sock* s) {
        size_t buf_size = s->getMaxSize();
        auto addr = s->getAddr();
        string addrStr(addr->getAddrStr());

        cout << "New connection: " << addr->getAddrStr() << ":" << addr->getPort() << endl;

        // Callback for when server receives message on new socket if command is sent execute that command
        s->onMessageReceived = [s, addrStr, &workingDIR, buf_size](string message) {
            cout << "[" << addrStr << "]: " << message << endl;
            string ret;
            // Close the socket 
            if (message == "DONE") {
                string r = "EXIT";
                size_t m_len = strlen(r.c_str());
                s->sendall(r);
            } 

            // Print the working directory
            else if(message == "PWD") {
                ret = workingDIR;
                s->sendall(ret);
            }

            // Change working directory
            else if(message.substr(0, 2) == "CD") {
                // If directory = .. move up a directory
                if (message.substr(3, message.size()) == ".."){
                    for (auto it = workingDIR.end(); it != workingDIR.begin(); it--) {
                        if (*it == '/') {
                            workingDIR.erase(it, workingDIR.end());
                            break;
                        }
                    }
                    s->sendall(workingDIR);
                } 
                // Check if string passed is a directory and change
                else {
                    workingDIR.append("/");
                    if (fs::is_directory(workingDIR + message.substr(3, message.size()))) {
                        workingDIR.append(message.substr(3, message.size()));
                        ret = workingDIR;
                    } else {
                        ret = "Not a folder";
                    }
                }

                s->sendall(ret);
            }

            // Return the directory listing
            else if(message == "DIR") {
                stringstream ss;
                fs::path p(workingDIR);

                FileTree ft(p);
                auto rootp = ft.getRootPath();

                // function to get the listing, push strings to a string stream
                auto action = [rootp, &ss](shared_ptr<FileNode> node) {
                    if (node->path == rootp) {
                        fs::path p(node->path);
                        ss << static_cast<string>(p.filename()) << endl;
                    } else {
                        auto parent = node->getParent();
                        string name;
                        while (parent->getParent() != nullptr) {
                            name = static_cast<string>(parent->path.filename()) + "/" + name;
                            parent = parent->getParent();
                        }
                        ss << name << static_cast<string>(node->path.filename()) << endl;
                    }
                };

                ft.fileAction(action);

                // move contents from stringstream to vector
                vector<string> st(0);
                string tmp;
                while (getline(ss, tmp)) {
                    tmp.append("\n");
                    st.push_back(tmp);
                }

                // send message in chunks of buf_size, or all at once if smaller than buf_size
                if (st.size() > buf_size){
                    int tmp_size = 0;
                    string send;
                    auto it = st.begin();
                    for (; it != st.end(); it++){
                        if (tmp_size < buf_size){
                            send.append(*it);
                            tmp_size += it->size();
                        } else {
                            s->sendall(send);
                            send.clear();
                            tmp_size = 0;
                        }
                    }
                    if (!send.empty()) {
                        s->sendall(send);
                    }
                } else {
                    string send;
                    send.resize(st.size());
                    for (auto it = st.begin(); it != st.end(); it++) {
                        send.append(*it);
                    }
                    s->sendall(send);
                }
            }

            // Send a file
            else if (message.substr(0,3) == "RET") {
                string filename = workingDIR + "/" + message.substr(4,message.size());
                cout << filename << endl;

                // open the file and init buffer
                ifstream file;
                file.open(filename, ifstream::in | ifstream::binary);
                vector<char> out_buf(0);

                if (!file.good()) {
                    s->sendall("File not found");
                } else {
                    // copy buffer into vector
                    if (!file.eof()) {
                        file.seekg(0, std::ios_base::end);
                        std::streampos fileSize(file.tellg());
                        out_buf.resize(fileSize);

                        file.seekg(0, std::ios_base::beg);
                        file.read(&out_buf.front(), fileSize);
                    }

                    vector<char> compressed(0);
                    buffer::compress_vector(out_buf, compressed);

                    // send file in chunks
                    if (compressed.size() > buf_size) {
                        int sent = 0;
                        int left = compressed.size();
                        auto it = compressed.begin();
                        while (sent < compressed.size()) {
                            if (left < buf_size) break;
                            vector<char> tmp_buf(0);
                            tmp_buf.resize(buf_size);
                            for (int i = 0; i < buf_size; i++) {
                                tmp_buf[i] = *it;
                                it++;
                            }

                            s->sendall(tmp_buf.data(), buf_size);
                            sent += buf_size;
                            left -= buf_size;
                        }
                        vector<char> tmp_buf(0);
                        tmp_buf.resize(left);
                        for (int i = 0; it != compressed.end(); i++) {
                            tmp_buf[i] = *it;
                            it++;
                        }
                        s->sendall(tmp_buf.data(), left);
                    } else {
                        s->sendall(compressed.data(), compressed.size());
                    }
                }
                file.close();
            }
            else {
                ret = "Recieved: \"" + message + "\"";
                ret.shrink_to_fit();
                s->sendall(ret);
            }
        };

        // Callback for when remote socket closes
        s->onSocketClosed = [addrStr](int code) {
            cout << "Socket closed: " << addrStr << " => " << code << endl;
            cout << flush;
        };
    };

    // Callback for when server binds to address and port
    server.bind("127.0.0.1", 8888, [](int code, string message) {
        cout << code << " : " << message << endl;
    });

    // Begin non-blocking acceptor loop
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
