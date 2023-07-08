#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "address.hpp"
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/fcntl.h>
#include <streambuf>
#include <vector>
#include <functional>
#include <thread>
#include <cstdlib>
#include <cstdio>
#include "buffer.hpp"

#define UNUSED(expr){ (void)(expr); }
#define ON_ERR std::function<void(int, std::string)> onError = [](int err_code, std::string err_message){UNUSED(err_code); UNUSED(err_message)}

class sock  {
public:
    /**
     * Callback for when socket recieves a message
    */
    std::function<void(std::string)> onMessageReceived;

    std::function<void(std::vector<char>&)> onRawReceived;
    /**
     * Callback for when socket successfully sends a message
    */
    std::function<void(std::string)> onMessageSent;
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

    sock(sock&& other) noexcept : sockfd(std::move(other.sockfd)), buf_size(std::move(other.buf_size)), addr(std::move(other.addr)), onMessageReceived(std::move(other.onMessageReceived)), onMessageSent(std::move(other.onMessageSent)), onSocketClosed(std::move(other.onSocketClosed)) {
        other.sockfd = -1;
        other.addr = nullptr;
        
    }

    ~sock() {
        if (addr != nullptr) {
            delete addr;
        }
    }

    void setMaxSize(int size) {
        buf_size = size;
    }

    std::size_t getMaxSize() const {
        return buf_size;
    }

    /**
     * Close the socket
    */
    void close() const {
        ::close(sockfd);
        if (onSocketClosed)
            onSocketClosed(errno);
    }

    /**
     * Assign address to socket
     * 
     * @param ad sys primative representing address info
    */
    void createAddr(sockaddr_in const &ad) {
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ad.sin_addr), ipStr, INET_ADDRSTRLEN);
        std::string ip(ipStr);
        addr = new address_v4(0, (uint16_t)ad.sin_port, ip.c_str());
        addr->freeInfo();
    }

    /**
     * Get pointer to address object
     * 
     * @return pointer to address object of socket
    */
    address_v4* getAddr() {

        return addr;
    }

    /**
     * Set socket to connect to endpoint, start new thread to send and recieve
     * 
     * @param host IP address to connect to 
     * @param port port on host to connect to
     * @param onConnected function callback on successfull connection
     * @param ON_ERR function callback on error
    */
    void connect(const char* host, uint16_t port, std::function<void(address_v4*)> const& onConnected, ON_ERR) {
        addr = new address_v4(0, port, host);
        auto info = addr->getInfo(onError);

        for (p = info; p != nullptr; p = p->ai_next) {
            if (p->ai_family == AF_INET) {
                memcpy((void *)&(addr->address), (void*)p->ai_addr, sizeof(sockaddr_in));
                break;
            }
        }
        addr->freeInfo();

        auto ip = (uint32_t)addr->address.sin_addr.s_addr;

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
     * 
     * @param buffer data to send
     * @param len size of data
     * @return size_t bytes sent
    */
    std::size_t send(char* buffer, size_t len) const {
        size_t bytes_sent;
        
        bytes_sent = ::send(sockfd, buffer, len, 0);

        
        return bytes_sent;
    }

    /**
     * Receive data on the socket once
     * 
     * @param buffer container to recieve data into
     * @param size max ammount of data to receive from socket
     * @return size_t bytes received
    */
    std::size_t receive(char* buffer, std::size_t max_size) const {
        size_t bytes_recived;


        bytes_recived = recv(sockfd, buffer, max_size, 0);
        
        return bytes_recived;
    }

    /**
     * Send data on socket until buffer is empty
     * 
     * @param buffer data to be sent
    */
    void sendall(std::string buffer) const
    {
        int sent = 0;
        int n;

        unsigned long bytes_left = buffer.size();
        unsigned long len = buffer.size();


        while (sent < len) {
            n = ::send(sockfd, buffer.data()+sent, bytes_left, 0);
            if (n == -1) { break; }
            sent += n;
            bytes_left -= n;
        }

        if (onMessageSent) {
            onMessageSent(buffer);
        }
    }

    /**
     * Overloaded send all for raw buffer
     * 
     * @param buffer data to be sent
     * @param len size of data
    */
    void sendall(const char* buffer, std::size_t len) const
    {
        int sent = 0;
        int n;

        unsigned long bytes_left = len;

        while (sent < len) {
            n = ::send(sockfd, buffer+sent, bytes_left, 0);
            if (n == -1) { break; }
            sent += n;
            bytes_left -= n;
        }

        if (onMessageSent) {
            std::string message(buffer, len);
            onMessageSent(message);
        }
    }

    /**
     * Create a new thread to send and receive on 
    */
    void spawnTask(bool dele = false, ON_ERR) {
        this->del = dele;
        std::thread t(createTask, std::move(this), onError);
        t.detach();
    }

    bool del = false;

protected:
    /**
     * Wait for data to appear in socket, should be run in a new thread
    */
    static void createTask(sock* s, ON_ERR){
        std::size_t buf_size = s->getMaxSize();
        long len = 0;

        std::vector<char> in_buf(0);
        int total_len = 0;

        
        while (len >= 0) {
            auto* buffer = new char[buf_size];
            len = recv(s->sockfd, buffer, buf_size, 0);
            // Remote socket closed
            if (len == 0) {
                delete[] buffer;
                s->close();
                s->del = true;
                break;
            }
            // Error 
            else if (len < 0) {
                onError(errno, "Failed to Recieve");
                delete[] buffer;
                s->close();
                s->del = true;
                break;
            }
            // Data received 
            else {
                buffer::add_buffer_to_vector(in_buf, buffer, len);
                total_len += len;

                // Stop receive blocking to check for data
                fcntl(s->sockfd, F_SETFL, O_NONBLOCK);
                if (!(recv(s->sockfd, buffer, buf_size, MSG_PEEK) >= 1)){
                    // No more data to rec run callback
                    std::string data(in_buf.data(), total_len);
                    if (s->onRawReceived)
                        s->onRawReceived(in_buf);
                    
                    if (s->onMessageReceived)
                        s->onMessageReceived(data);

                    in_buf = std::vector<char>(0);
                    total_len = 0;
                }
                fcntl(s->sockfd, F_SETFL, O_FSYNC);
                delete[] buffer;
            }
        }
        // If we're done with the socket, delete it
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
