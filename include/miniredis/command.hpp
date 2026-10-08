#pragma once

#include "miniredis/protocol.hpp"
#include "miniredis/store.hpp"
#include <string>

namespace miniredis {

/**
 * @brief Command execution & dispatch module.
 * 
 * Takes a parsed Command (name + arguments) and a reference to the Store,
 * validates argument count, executes the appropriate operation on Store,
 * and serializes the result into Redis wire format via Protocol serializers.
 */
class CommandDispatcher {
public:
    // Main entry point for command execution
    // Dispatches to the appropriate private handler based on cmd.name
    static std::string execute(const Command& cmd, Store& store);

private:
    // Individual command handlers
    // Each handler validates arguments, interacts with Store, and serializes response

    // PING [message]
    static std::string handle_ping(const Command& cmd);

    // SET key value
    static std::string handle_set(const Command& cmd, Store& store);

    // GET key
    static std::string handle_get(const Command& cmd, const Store& store);

    // DEL key
    static std::string handle_del(const Command& cmd, Store& store);

    // EXISTS key
    static std::string handle_exists(const Command& cmd, const Store& store);

    // Callable helper to safely execute operations with unified exception handling
    template <typename Func>
    static std::string safe_execute(const std::string& op_label, Func&& op) {
        try {
            return op();
        } catch (const std::exception& e) {
            return Protocol::serialize_error(op_label + " failed: " + e.what());
        } catch (...) {
            return Protocol::serialize_error(op_label + " failed");
        }
    }
};

} // namespace miniredis
