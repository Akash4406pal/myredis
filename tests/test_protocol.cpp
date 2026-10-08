#include <catch2/catch.hpp>
#include "miniredis/protocol.hpp"

using namespace miniredis;

TEST_CASE("Protocol Parsing", "[protocol]") {
    SECTION("Parse empty or whitespace line returns nullopt") {
        auto cmd = Protocol::parse("");
        CHECK_FALSE(bool(cmd));

        auto cmd2 = Protocol::parse("   \r\n");
        CHECK_FALSE(bool(cmd2));
    }

    SECTION("Parse PING command with no arguments") {
        auto cmd = Protocol::parse("PING\r\n");
        REQUIRE(bool(cmd));
        CHECK(cmd->name == "PING");
        CHECK(cmd->args.empty());
    }

    SECTION("Parse SET command with arguments and case insensitivity") {
        auto cmd = Protocol::parse("set   key1   value1\r\n");
        REQUIRE(bool(cmd));
        CHECK(cmd->name == "SET");
        REQUIRE(cmd->args.size() == 2);
        CHECK(cmd->args[0] == "key1");
        CHECK(cmd->args[1] == "value1");
    }

    SECTION("Parse GET command") {
        auto cmd = Protocol::parse("GET key1\r\n");
        REQUIRE(bool(cmd));
        CHECK(cmd->name == "GET");
        REQUIRE(cmd->args.size() == 1);
        CHECK(cmd->args[0] == "key1");
    }
}

TEST_CASE("Protocol Serialization", "[protocol]") {
    SECTION("Simple String serialization") {
        auto simple_string = Protocol::serialize_simple_string("OK");
        CHECK(simple_string == "OK\r\n");
    }

    SECTION("Bulk String serialization") {
        auto bulk_string = Protocol::serialize_bulk_string("hello");
        CHECK(bulk_string == "hello\r\n");
    }

    SECTION("Null serialization") {
        auto null_string = Protocol::serialize_null();
        CHECK(null_string == "(nil)\r\n");
    }

    SECTION("Integer serialization") {
        auto integer = Protocol::serialize_integer(1);
        CHECK(integer == "1\r\n");
    }

    SECTION("Error serialization") {
        auto error = Protocol::serialize_error("unknown command");
        CHECK(error == "ERR unknown command\r\n");
    }
}
