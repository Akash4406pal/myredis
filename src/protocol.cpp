// protocol.cpp
#include "miniredis/protocol.hpp"

#include <sstream>
#include <algorithm>
#include <cctype>

namespace miniredis {
namespace Protocol {

std::optional<Command> parse(const std::string& raw_input) {
    std::istringstream iss(raw_input);
    std::string token;

    if (!(iss >> token)) {
        // Empty or whitespace-only input
        return std::nullopt;
    }
    // Convert command name to uppercase for case-insensitivity
    std::transform(token.begin(), token.end(), token.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    Command cmd;
    cmd.name = token;

    // Collect remaining arguments
    while (iss >> token) {
        cmd.args.push_back(token);
    }

    return cmd;
}

std::string serialize_simple_string(const std::string& msg) {
    return msg + "\r\n";
}

std::string serialize_bulk_string(const std::string& val) {
    return val + "\r\n";
}

std::string serialize_null() {
    return "(nil)\r\n";
}

std::string serialize_integer(int64_t val) {
    return std::to_string(val) + "\r\n";
}

std::string serialize_error(const std::string& msg) {
    return "ERR " + msg + "\r\n";
}

} // namespace Protocol
} // namespace miniredis
