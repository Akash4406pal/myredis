#include "miniredis/store.hpp"

// SET key value
// Stores or updates the key-value pair in the hash map.
// Returns true upon overwriting an existing key.
bool Store::set(const std::string& key, const std::string& value) {
    auto it = storage_.find(key);
    storage_[key] = value;
    return it != storage_.end();
}

// GET key
// Looks up the key in the hash map.
// Returns std::optional with the value if found, or std::nullopt if the key does not exist.
std::optional<std::string> Store::get(const std::string& key) const {
    auto it = storage_.find(key);
    if (it != storage_.end()) {
        return it->second;
    }
    return std::nullopt;
}

// DEL key
// Removes the key from the hash map.
// Returns true if the key existed and was deleted (erase count > 0), false otherwise.
bool Store::del(const std::string& key) {
    return storage_.erase(key) > 0;
}

// EXISTS key
// Checks whether the key is present in the hash map without retrieving the value.
// Returns true if key exists, false otherwise.
bool Store::exists(const std::string& key) const {
    return storage_.count(key) > 0;
    // Alternatively in C++20: return storage_.contains(key);
}
