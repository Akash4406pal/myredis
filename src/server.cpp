// server.cpp
#include "miniredis/server.hpp"
#include "miniredis/protocol.hpp"
#include "miniredis/command.hpp"

#include <iostream>
#include <vector>
#include <cstring>

namespace miniredis {

Server::Server(int port, Store& store)
    : port_(port), store_(store) {}

Server::~Server() {
    stop();
    cleanup_network();
}

bool Server::init_network() {
#ifdef _WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        std::cerr << "[Error] WSAStartup failed: " << result << "\n";
        return false;
    }
#endif
    return true;
}

void Server::cleanup_network() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void Server::close_socket(socket_t sock) {
    if (sock != INVALID_SOCKET_VALUE) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }
}

bool Server::setup_server_socket() {
    if (!init_network()) {
        return false;
    }

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ == INVALID_SOCKET_VALUE) {
        std::cerr << "[Error] Failed to create socket\n";
        return false;
    }

    // Set SO_REUSEADDR to avoid address in use errors on restart
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR_CODE) {
        std::cerr << "[Error] Failed to bind to port " << port_ << "\n";
        close_socket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
        return false;
    }

    if (listen(server_fd_, SOMAXCONN) == SOCKET_ERROR_CODE) {
        std::cerr << "[Error] Listen failed\n";
        close_socket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
        return false;
    }

    return true;
}

void Server::run() {
    if (!setup_server_socket()) {
        return;
    }

    running_ = true;
    std::cout << "[Server] MiniRedis listening on port " << port_ << "...\n";

    while (running_) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        socket_t client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET_VALUE) {
            if (!running_) break;
            std::cerr << "[Warning] Accept failed\n";
            continue;
        }

        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        std::cout << "[Client Connected] " << client_ip << ":" << ntohs(client_addr.sin_port) << "\n";

        // Handle client connection
        handle_client(client_fd);

        close_socket(client_fd);
        std::cout << "[Client Disconnected] " << client_ip << "\n";
    }
}

void Server::stop() {
    if (running_) {
        running_ = false;
        close_socket(server_fd_);
        server_fd_ = INVALID_SOCKET_VALUE;
    }
}

bool Server::send_all(socket_t client_fd, const std::string& data) {
    size_t total_sent = 0;
    while (total_sent < data.size()) {
        int sent = send(client_fd, data.c_str() + total_sent, static_cast<int>(data.size() - total_sent), 0);
        if (sent <= 0) {
            return false;
        }
        total_sent += sent;
    }
    return true;
}

void Server::handle_client(socket_t client_fd) {
    char buffer[1024];
    std::string stream_buffer;

    while (running_) {
        int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            break; // Client disconnected or error
        }

        buffer[bytes_read] = '\0';
        stream_buffer.append(buffer, bytes_read);

        // Process full lines terminated by '\n'
        size_t newline_pos;
        while ((newline_pos = stream_buffer.find('\n')) != std::string::npos) {
            std::string line = stream_buffer.substr(0, newline_pos);
            stream_buffer.erase(0, newline_pos + 1);

            // Strip trailing '\r' if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.empty()) {
                continue;
            }

            std::cout << "[Command Received] " << line << "\n";

            auto parsed_cmd = Protocol::parse(line);
            std::string response;

            if (!parsed_cmd) {
                response = Protocol::serialize_error("empty or invalid command");
            } else {
                response = CommandDispatcher::execute(*parsed_cmd, store_);
            }

            if (!send_all(client_fd, response)) {
                return; // Failed to send, terminate connection
            }
        }
    }
}

} // namespace miniredis
