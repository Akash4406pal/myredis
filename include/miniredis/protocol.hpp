#pragma once

#include <string>
#include <vector>
#include <cstdint>

#if __has_include(<optional>) && (!defined(__GNUC__) || __GNUC__ >= 7)
#include <optional>
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
namespace std {
    using experimental::optional;
    using experimental::nullopt;
}
#endif

namespace miniredis {

struct Command {
    std::string name;               // e.g. "SET", "GET", "PING"
    std::vector<std::string> args;  // e.g. ["foo", "bar"]
};

namespace Protocol {
    // Parser: converts incoming raw line into a Command struct
    std::optional<Command> parse(const std::string& raw_input);

    // Encoders: serialize responses into plain-text wire format
    std::string serialize_simple_string(const std::string& msg); // OK\r\n or PONG\r\n
    std::string serialize_bulk_string(const std::string& val);   // foo\r\n
    std::string serialize_null();                                // (nil)\r\n
    std::string serialize_integer(int64_t val);                  // 1\r\n or 0\r\n
    std::string serialize_error(const std::string& msg);         // ERR <msg>\r\n
}

} // namespace miniredis
