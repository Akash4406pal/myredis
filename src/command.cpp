#include "miniredis/command.hpp"
#include <exception>
#include <string>

namespace miniredis {

std::string CommandDispatcher::execute(const Command& cmd, Store& store) {
    //Checking cmd.name against supported commands:
    if(cmd.name == "PING") {//       - "PING"   -> call handle_ping(cmd)
        return handle_ping(cmd);
    }
    else if(cmd.name == "SET") { //       - "SET"    -> call handle_set(cmd, store)
        return handle_set(cmd, store);
    } 
    else if(cmd.name == "GET") {//       - "GET"    -> call handle_get(cmd, store)
        return handle_get(cmd, store);
    } 
    else if(cmd.name == "DEL") {//       - "DEL"    -> call handle_del(cmd, store)
        return handle_del(cmd, store);
    } 
    else if(cmd.name == "EXISTS") {//       - "EXISTS" -> call handle_exists(cmd, store)
        return handle_exists(cmd, store);
    }
    else { // If cmd.name does not match any known command, return error
        return Protocol::serialize_error("unknown command '" + cmd.name + "'");
    }
}

std::string CommandDispatcher::handle_ping(const Command& cmd) {
    if(cmd.args.size() > 1) {//       PING accepts either 0 arguments or 1 argument (echo message).
        return Protocol::serialize_error("wrong number of arguments for 'ping' command");
    }
    if(cmd.args.empty()) {//      If no arguments provided, return "PONG"
        return Protocol::serialize_simple_string("PONG");
    } 
    else {//    If 1 argument provided, return that argument as a bulk string
        return Protocol::serialize_bulk_string(cmd.args[0]);
    }
}

std::string CommandDispatcher::handle_set(const Command& cmd, Store& store) {
    if (cmd.args.size() != 2) {
        return Protocol::serialize_error("wrong number of arguments for 'set' command");
    }
    const auto& key = cmd.args[0];
    const auto& value = cmd.args[1];

    return safe_execute("set", [&]() {
        store.set(key, value);
        return Protocol::serialize_simple_string("OK");
    });
}

std::string CommandDispatcher::handle_get(const Command& cmd, const Store& store) {
    if (cmd.args.size() != 1) {
        return Protocol::serialize_error("wrong number of arguments for 'get' command");
    }
    const auto& key = cmd.args[0];

    return safe_execute("get", [&]() {
        auto value = store.get(key);
        if (value) {
            return Protocol::serialize_bulk_string(*value);
        }
        return Protocol::serialize_null();
    });
}

std::string CommandDispatcher::handle_del(const Command& cmd, Store& store) {
    if (cmd.args.empty()) {
        return Protocol::serialize_error("wrong number of arguments for 'del' command");
    }

    return safe_execute("del", [&]() {
        int64_t count = 0;
        for (const auto& key : cmd.args) {
            if (store.del(key)) {
                count++;
            }
        }
        return Protocol::serialize_integer(count);
    });
}

std::string CommandDispatcher::handle_exists(const Command& cmd, const Store& store) {
    if (cmd.args.empty()) {
        return Protocol::serialize_error("wrong number of arguments for 'exists' command");
    }

    return safe_execute("exists", [&]() {
        int64_t count = 0;
        for (const auto& key : cmd.args) {
            if (store.exists(key)) {
                count++;
            }
        }
        return Protocol::serialize_integer(count);
    });
}

} // namespace miniredis
