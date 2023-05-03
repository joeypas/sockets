#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory>
#include <string>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>


class address_v4 {
public:
    sockaddr_in address;

    address_v4() {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
    }

    address_v4(int socktype, uint16_t port, const char* addr = "") {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        if (socktype == 0) hints.ai_socktype = SOCK_STREAM;
        else if (socktype == 1) hints.ai_socktype = SOCK_DGRAM;
        if (strlen(addr) == 1) {
            hints.ai_flags = AI_PASSIVE;
            this->addr = "localhost";
        }
        else {
            hints.ai_flags = 0;
            this->addr = addr;
        }
        this->port = port;
    }

    void freeInfo() {
        freeaddrinfo(res);
    }

    addrinfo* getInfo(std::function<void(int, std::string)> onError) {
        //char p[12];
        //snprintf(p, sizeof(port), "%d", port);

        if (addr.empty()) {
            int s = getaddrinfo(NULL, NULL, &hints, &res);
        } else {
            int s = getaddrinfo(addr.c_str(), NULL, &hints, &res);
        }
        
        if (s != 0){
            onError(errno, "Invalid Address");
            return nullptr;
        };
        return res;
    }

    void setAddr(std::function<void(int, std::string)> onError) {
        int e = inet_pton(hints.ai_family, addr.c_str(), &address.sin_addr);
        if (e == -1) {
            onError(errno, "Invalid Host");
            return;
        } else if (e == 0) {
            onError(errno, "Error!");
            return;
        }
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        res = &hints;
    }

    std::string getAddrStr() {
        return addr;
    }

    int getPort() {
        return port;
    }


private:
    void *get_in_addr(struct sockaddr *sa) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    struct addrinfo hints;
    struct addrinfo *res;

    std::string addr;
    char addrstore[INET_ADDRSTRLEN];
    uint16_t port;
    int s;
};


#endif
