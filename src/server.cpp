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
    char* cwd = new char[FILENAME_MAX];
    getcwd(cwd, FILENAME_MAX);

    string workingDIR(cwd);
    delete[] cwd;

    acceptor server([](int code, std::string message){
        cout << "Server: (" << code << " : " << message << endl;
    });

    server.onNewConnection = [&workingDIR](sock* s) {
        auto addr = s->getAddr();
        string addrStr(addr->getAddrStr());

        cout << "New connection: " << addr->getAddrStr() << ":" << addr->getPort() << endl;

        s->onMessageReceived = [s, addrStr, &workingDIR](string message) {
            cout << "[" << addrStr << "]: " << message << endl;
            string ret;
            if (message == "DONE") {
                string r = "EXIT";
                size_t m_len = strlen(r.c_str());
                s->sendall(r);
            } 
            else if(message == "PWD") {
                ret = workingDIR;
                s->sendall(ret);
            }
            else if(message.substr(0, 2) == "CD") {
                if (message.substr(3, message.size()) == ".."){
                    for (auto it = workingDIR.end(); it != workingDIR.begin(); it--) {
                        if (*it == '/') {
                            workingDIR.erase(it, workingDIR.end());
                            break;
                        }
                    }
                    s->sendall(workingDIR);
                } else {
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
            else if(message == "DIR") {
                string st;
                stringstream ss;
                fs::path p(workingDIR);

                FileTree ft(p);
                auto rootp = ft.getRootPath();

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

                std::string tmp;
                while (getline(ss, tmp)) {
                    st += tmp;
                    st.append("\n");
                }
                s->sendall(st);
            }
            else {
                ret = "Recieved: \"" + message + "\"";
                ret.shrink_to_fit();
                s->sendall(ret);
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