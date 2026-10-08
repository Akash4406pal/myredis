#pragma once


#include <string>
#include <unordered_map>

#if __has_include(<optional>) && (!defined(__GNUC__) || __GNUC__ >= 7)
#include <optional>
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
namespace std {
    using experimental::optional;
    using experimental::nullopt;
}
#endif
class Store {
public:
    Store() = default;
    ~Store() = default;
    bool set(const std::string& key, const std::string& value);// AS set returns true if the key was set successfully, false otherwise
    std::optional<std::string> get(const std::string& key) const; // AS get returns an optional string, which will contain the value if the key exists, or std::nullopt if it does not const represents that the function does not modify the state of the Store object
    bool del(const std::string& key);// AS del returns true if the key was deleted successfully, false otherwise
    bool exists(const std::string& key) const;// AS exists returns true if the key exists, false otherwise and const at last cause it can't change the state of the store
private:
    std::unordered_map<std::string, std::string>storage_;
};