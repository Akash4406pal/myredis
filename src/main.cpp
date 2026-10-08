// main.cpp
#include "miniredis/store.hpp"
#include "miniredis/server.hpp"
#include <iostream>
#include <csignal>

static miniredis::Server* g_server = nullptr;

void signal_handler(int) {
    std::cout << "\n[Server] Shutting down gracefully...\n";
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 6380; // Default port
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "Invalid port provided. Using default: 6380\n";
            port = 6380;
        }
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    Store store;
    miniredis::Server server(port, store);
    g_server = &server;

    server.run();

    return 0;
}
