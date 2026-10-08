#include <catch2/catch.hpp>
#include "miniredis/command.hpp"
#include "miniredis/store.hpp"
#include "miniredis/protocol.hpp"

using namespace miniredis;

TEST_CASE("Command Dispatcher - PING", "[command]") {
    Store store;

    SECTION("PING without args returns PONG\r\n") {
        Command cmd{"PING", {}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "PONG\r\n");
    }

    SECTION("PING with argument returns message as plain string") {
        Command cmd{"PING", {"hello"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "hello\r\n");
    }

    SECTION("PING with too many arguments returns error") {
        Command cmd{"PING", {"arg1", "arg2"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result.rfind("ERR", 0) == 0);
    }
}

TEST_CASE("Command Dispatcher - SET and GET", "[command]") {
    Store store;

    SECTION("SET key value returns OK\r\n and updates store") {
        Command cmd{"SET", {"mykey", "myval"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "OK\r\n");
        CHECK(*store.get("mykey") == "myval");
    }

    SECTION("GET existing key returns plain string") {
        store.set("mykey", "myval");
        Command cmd{"GET", {"mykey"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "myval\r\n");
    }

    SECTION("GET non-existing key returns null (nil)\r\n") {
        Command cmd{"GET", {"nokey"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "(nil)\r\n");
    }

    SECTION("SET/GET invalid argument counts return error") {
        Command cmd{"SET", {"mykey"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result.rfind("ERR", 0) == 0);
    }
}

TEST_CASE("Command Dispatcher - DEL and EXISTS", "[command]") {
    Store store;

    SECTION("EXISTS returns 1 for present key and 0 for missing key") {
        Command cmd{"EXISTS", {"mykey"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "0\r\n");

        store.set("mykey", "myval");
        result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "1\r\n");
    }

    SECTION("EXISTS with multiple keys returns total count") {
        store.set("k1", "v1");
        store.set("k2", "v2");
        Command cmd{"EXISTS", {"k1", "k2", "k3"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "2\r\n");
    }

    SECTION("EXISTS with no args returns error") {
        Command cmd{"EXISTS", {}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result.rfind("ERR", 0) == 0);
    }

    SECTION("DEL removes key and returns count 1, subsequent DEL returns 0") {
        store.set("mykey", "myval");
        Command cmd{"DEL", {"mykey"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "1\r\n");
        CHECK(store.exists("mykey") == false);

        result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "0\r\n");
    }

    SECTION("DEL with multiple keys returns count of deleted keys") {
        store.set("k1", "v1");
        store.set("k2", "v2");
        Command cmd{"DEL", {"k1", "k2", "k3"}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result == "2\r\n");
        CHECK_FALSE(store.exists("k1"));
        CHECK_FALSE(store.exists("k2"));
    }

    SECTION("DEL with no args returns error") {
        Command cmd{"DEL", {}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result.rfind("ERR", 0) == 0);
    }
}

TEST_CASE("Command Dispatcher - Unknown Command", "[command]") {
    Store store;

    SECTION("Unknown command returns error") {
        Command cmd{"FOOBAR", {}};
        std::string result = CommandDispatcher::execute(cmd, store);
        CHECK(result.rfind("ERR unknown command", 0) == 0);
    }
}

