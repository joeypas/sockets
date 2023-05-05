#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "address.hpp"
#include <sys/socket.h>
#include <sys/poll.h>
#include <streambuf>
#include <vector>
#include <functional>
#include <thread>

#define UNUSED(expr){ (void)(expr); }
#define ON_ERR std::function<void(int, std::string)> onError = [](int err_code, std::string err_message){UNUSED(err_code); UNUSED(err_message)}

class sock  {
public:
    /**
     * Callback for when socket recieves a message
    */
    std::function<void(char*, std::size_t)> onMessageReceived;
    /**
     * Callback for when socket successfully sends a message
    */
    std::function<void(char*, std::size_t)> onMessageSent;
    /**
     * Callback for when socket closes 
    */
    std::function<void(int)> onSocketClosed;

    /**
     * Constructor
    */
    sock(ON_ERR, int sock_id = -1) {
        if (sock_id == -1) {
            sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                onError(errno, "Socket: create.");
            }
        } else {
            sockfd = sock_id;
        }
        buf_size = 0x1000;

    }

    /**
     * Close the socket
    */
    void close() {
        ::close(sockfd);
        if (onSocketClosed)
            onSocketClosed(errno);
    }

    /**
     * Assign address to socket
    */
    void createAddr(sockaddr_in &a) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(a.sin_addr), ipStr, INET_ADDRSTRLEN);
        std::string ip(ipStr);
        addr = new address_v4(0, (uint16_t)a.sin_port, ip.c_str());
        addr->freeInfo();
    }

    /**
     * Get pointer to address object
    */
    address_v4* getAddr() {

        return addr;
    }

    /**
     * Set socket to connect to endpoint, start new thread to send and recieve
    */
    void connect(const char* host, uint16_t port, std::function<void(address_v4*)> onConnected, ON_ERR) {
        addr = new address_v4(0, port, host);
        auto info = addr->getInfo(onError);

        for (p = info; p != NULL; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                memcpy((void *)&(addr->address), (void*)p->ai_addr, sizeof(sockaddr_in));
                break;
            }
        }
        addr->freeInfo();

        uint32_t ip = (uint32_t)addr->address.sin_addr.s_addr;

        addr->address.sin_family = AF_INET;
        addr->address.sin_port = htons(port);
        addr->address.sin_addr.s_addr = ip;


        int s = ::connect(sockfd, (const sockaddr*)&addr->address, sizeof(sockaddr_in));
        if (s == -1) {
            onError(errno, "Couldn't connect");
            return;
        }

        onConnected(addr);


        std::thread t(createTask, this, onError);
        t.detach();
    } 
    
    /**
     * Send data on the socket once
    */
    std::size_t send(char* buffer, size_t len) {
        size_t buf_size, bytes_sent;
        
        buf_size = len;
        
        bytes_sent = ::send(sockfd, buffer, buf_size, 0);

        
        return bytes_sent;
    }

    /**
     * Receive data on the socket once
    */
    std::size_t receive(char* buffer, std::size_t s) {
        size_t bytes_recived;

        bytes_recived = recv(sockfd, buffer, s, 0);
        
        return bytes_recived;
    }

    /**
     * Send data on socket until buffer is empty
    */
    void sendall(char* buffer, size_t len)
    {
        int sent = 0;
        int bytes_left = len;
        int n;

        while (sent < strlen(buffer)) {
            n = ::send(sockfd, buffer+sent, bytes_left, 0);
            if (n == -1) { break; }
            sent += n;
            bytes_left -= n;
        }

        if (onMessageSent)
            onMessageSent(buffer, sent);
    }

    /**
     * Create a new thread to send and receive on 
    */
    void spawnTask(bool del = false, ON_ERR) {
        this->del = del;
        std::thread t(createTask, std::move(this), onError);
        t.detach();
    }

    bool del = false;

protected:
    /**
     * Wait for data to appear in socket, should be run in a new thread
    */
    static void createTask(sock* s, ON_ERR){
        char buffer[0x1000];
        std::size_t len;
        
        len = recv(s->sockfd, buffer, 0x1000, 0);

        while (len >= 0) {
            if (len == 0) {
                s->close();
                s->del = true;
                break;
            } else if (len < 0) {
                onError(errno, "Failed to Recieve");
                s->close();
                s->del = true;
                break;
            } else {
                buffer[len] = '\0';
                s->onMessageReceived(buffer, len);
                len = recv(s->sockfd, buffer, 0x1000, 0);
            }
        }

        //s->close();
        //s->onSocketClosed(errno);
        if (s->del && s != nullptr) {
            delete s;
        } else {
            createTask(s);
        }
    }

    int sockfd;
    address_v4* addr;
    struct addrinfo *p;

    std::size_t buf_size;

};


#endif
