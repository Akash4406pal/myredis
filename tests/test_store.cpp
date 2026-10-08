#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "miniredis/store.hpp"

TEST_CASE("Store basic operations", "[store]") {
    Store store;

    SECTION("Set a fresh entry and Get it") {
        CHECK_FALSE(store.set("name", "alice"));
        auto result = store.get("name");
        REQUIRE(bool(result));
        CHECK(*result == "alice");
    }
    SECTION("Set an existing entry and Get it") {
        CHECK_FALSE(store.set("name", "alice"));
        REQUIRE(store.set("name", "bob"));
        auto result = store.get("name");
        REQUIRE(bool(result));
        CHECK(*result == "bob");
    }
    SECTION("Get non-existing key") {
        auto result = store.get("nonexistent");
        CHECK_FALSE(static_cast<bool>(result));
    }
    SECTION("Get empty value") {
        store.set("city", "");
        auto result = store.get("city");
        CHECK(static_cast<bool>(result));
        CHECK(*result == "");
    }

    SECTION("Exists") {
        REQUIRE_FALSE(store.exists("age"));
        store.set("age", "25");
        CHECK(store.exists("age"));
    }
    SECTION("Exists non-existing key") {
        REQUIRE_FALSE(store.exists("nonexistent"));
    }

    SECTION("Normal Delete") {
        REQUIRE_FALSE(store.exists("temp"));
        store.set("temp", "value");
        REQUIRE(store.exists("temp"));
        REQUIRE(store.del("temp"));
        CHECK_FALSE(store.exists("temp"));
    }
    SECTION("Delete non-existing key") {
        REQUIRE_FALSE(store.exists("nonexistent"));
        CHECK_FALSE(store.del("nonexistent"));
    }
}
