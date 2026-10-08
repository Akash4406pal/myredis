#pragma once

#include "miniredis/store.hpp"
#include <atomic>
#include <cstdint>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#define SOCKET_ERROR_CODE SOCKET_ERROR
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using socket_t = int;
#define INVALID_SOCKET_VALUE (-1)
#define SOCKET_ERROR_CODE (-1)
#endif

namespace miniredis {

class Server {
public:
    explicit Server(int port, Store& store);
    ~Server();

    // Start accepting connections (blocking loop)
    void run();

    // Signal server to stop
    void stop();

private:
    int port_;
    Store& store_;
    socket_t server_fd_{INVALID_SOCKET_VALUE};
    std::atomic<bool> running_{false};

    // Socket lifecycle helpers
    bool init_network();
    bool setup_server_socket();
    void close_socket(socket_t sock);
    void cleanup_network();

    // Client connection handling
    void handle_client(socket_t client_fd);
    bool send_all(socket_t client_fd, const std::string& data);
};

} // namespace miniredis
