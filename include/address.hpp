#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <memory>
#include <string>
#include <cstring>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

/**
 * Represents an IPV4 address
*/
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

    address_v4(address_v4&& other) noexcept : address(std::move(other.address)), hints(std::move(other.hints)), addr(std::move(other.addr)), port(std::move(other.port)) {
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
    }
        
    

    void freeInfo() {
        if (res != nullptr)
            freeaddrinfo(res);
    }

    addrinfo* getInfo(std::function<void(int, std::string)> const &onError) {

        if (addr.empty()) {
            int s = getaddrinfo(nullptr, nullptr, &hints, &res);
        } else {
            int s = getaddrinfo(addr.c_str(), nullptr, &hints, &res);
        }
        
        if (s != 0){
            onError(errno, "Invalid Address");
            return nullptr;
        };
        return res;
    }

    void setAddr(std::function<void(int, std::string)> const &onError) {
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
        if (res == nullptr) {
            getInfo(onError);
        }
    }

    std::string getAddrStr() const {
        return addr;
    }

    int getPort() const {
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
