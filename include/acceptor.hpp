#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP
#include "socket.hpp"

/**
 * Represents a socket that can accept new connections
*/
class acceptor : public sock {
public:
    /**
     * Callback function for new connections
     */
    std::function<void(sock*)> onNewConnection;

    // Remote socket info
    struct remote_sock {
        sockaddr_in remote_addr;
        socklen_t remote_size;
    };

    struct remote_sock rs;


    acceptor(ON_ERR, int backlog = 20) : sock(onError) {
        int opt = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));
        this->backlog = backlog;
    }

    /**
     * Bind to specified host/port 
     * 
     * @param host string containing host IP address
     * @param port port to bind to
     * @param ON_ERR error callback
    */
    void bind(const char* host, uint16_t port, ON_ERR) {
        addr = new address_v4(0, port, host);
        addr->setAddr(onError);
        addr->freeInfo();
        if (::bind(sockfd, (const sockaddr*)&addr->address, sizeof(addr->address)) == -1) {
            onError(errno, "Bind error");
        } 
    }

    /**
     * Start listening to socket
     * 
     * @return int status, -1 = error
    */
    int listen() {
        return ::listen(sockfd, backlog);
    }

    /**
     * Accept incoming connection and create a new socket
     * 
     * @return socket file descriptor
    */
    int accept() {
        sockaddr_in newInfo;
        socklen_t infoLength = sizeof(newInfo);
        int fd = ::accept(sockfd, (sockaddr*)&newInfo, &infoLength);

        rs.remote_addr = newInfo;
        rs.remote_size = infoLength;
        return fd;
    }

    /**
     * Start the accept loop on a new thread
     * 
     * @param ON_ERR error callback
    */
    void beginEventLoop(ON_ERR) {
        if (listen() == -1){
            onError(errno, "Failed to listen to the socket");
            return;
        }
        std::thread t(eventLoop, this, onError);
        t.detach();
    }

private:
    /**
     * Event loop for acceptor, run on it's own thread.
     * Accepts a connection and creates a new socket and thread
     * to send and receive data on.
    */
    static void eventLoop(acceptor* e, ON_ERR) {
        int s = -1;
        sock* newsock;
        while (true) {

            s = e->accept();
            if (s == -1) onError(errno, "Failed to accept connection");

            std::function<void(int, std::string)> errr = onError;

            newsock = new sock(errr, s);

            newsock->createAddr(e->rs.remote_addr);

            e->onNewConnection(newsock);
            newsock->spawnTask(false, onError);
        }
    }

    int backlog;
};
#endif