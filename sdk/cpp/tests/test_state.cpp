/**
 * Tests for quantflow::state module
 *
 * Provides persistent state management for bots across executions.
 */

#include <doctest/doctest.h>
#include <quantflow/state.h>
#include "helpers/test_helpers.hpp"
#include <algorithm>
#include <filesystem>

using namespace quantflow_test;
namespace fs = std::filesystem;

/**
 * RAII helper to set up a temp directory for state tests
 */
class StateContext {
public:
    StateContext() {
        // Create temp directory
        temp_dir_ = fs::temp_directory_path() / ("quantflow_state_test_" + std::to_string(rand()));
        fs::create_directories(temp_dir_);

        // Set environment variable (quantflow SDK reads STATE_DIR)
        setenv("STATE_DIR", temp_dir_.c_str(), 1);
    }

    ~StateContext() {
        // Unset env
        unsetenv("STATE_DIR");

        // Cleanup temp directory
        try {
            fs::remove_all(temp_dir_);
        } catch (...) {
            // Ignore cleanup errors in destructor
        }
    }

    // Non-copyable
    StateContext(const StateContext&) = delete;
    StateContext& operator=(const StateContext&) = delete;

private:
    fs::path temp_dir_;
};

TEST_SUITE("quantflow::state::set and get") {
    TEST_CASE("stores and retrieves an object") {
        StateContext ctx;

        nlohmann::json portfolio = {{"AAPL", 100}, {"GOOGL", 50}};
        quantflow::state::set("portfolio", portfolio);
        auto retrieved = quantflow::state::get("portfolio");

        REQUIRE(retrieved.has_value());
        CHECK((*retrieved)["AAPL"] == 100);
        CHECK((*retrieved)["GOOGL"] == 50);
    }

    TEST_CASE("stores and retrieves an array") {
        StateContext ctx;

        nlohmann::json prices = {45000.5, 45100.0, 45050.25};
        quantflow::state::set("prices", prices);
        auto retrieved = quantflow::state::get("prices");

        REQUIRE(retrieved.has_value());
        CHECK(retrieved->is_array());
        CHECK(retrieved->size() == 3);
        CHECK(std::abs((*retrieved)[0].get<double>() - 45000.5) < 0.01);
    }

    TEST_CASE("stores and retrieves a number") {
        StateContext ctx;

        quantflow::state::set("count", 42);
        auto retrieved = quantflow::state::get("count");

        REQUIRE(retrieved.has_value());
        CHECK(*retrieved == 42);
    }

    TEST_CASE("stores and retrieves a string") {
        StateContext ctx;

        quantflow::state::set("symbol", "BTC/USD");
        auto retrieved = quantflow::state::get("symbol");

        REQUIRE(retrieved.has_value());
        CHECK(*retrieved == "BTC/USD");
    }

    TEST_CASE("stores and retrieves a boolean") {
        StateContext ctx;

        quantflow::state::set("active", true);
        auto retrieved = quantflow::state::get("active");

        REQUIRE(retrieved.has_value());
        CHECK(*retrieved == true);
    }

    TEST_CASE("stores and retrieves null") {
        StateContext ctx;

        quantflow::state::set("nothing", nullptr);
        auto retrieved = quantflow::state::get("nothing");

        REQUIRE(retrieved.has_value());
        CHECK(retrieved->is_null());
    }
}

TEST_SUITE("quantflow::state::get with defaults") {
    TEST_CASE("returns nullopt for non-existent key") {
        StateContext ctx;

        auto result = quantflow::state::get("nonexistent");
        CHECK_FALSE(result.has_value());
    }

    TEST_CASE("get_or returns default for non-existent key") {
        StateContext ctx;

        nlohmann::json default_val = {{"default", true}};
        auto result = quantflow::state::get_or("nonexistent", default_val);
        CHECK(result["default"] == true);
    }

    TEST_CASE("get_or returns stored value when key exists") {
        StateContext ctx;

        quantflow::state::set("key", "actual");
        auto result = quantflow::state::get_or("key", "default");
        CHECK(result == "actual");
    }
}

TEST_SUITE("quantflow::state::exists") {
    TEST_CASE("returns true for existing key") {
        StateContext ctx;

        quantflow::state::set("exists_test", "value");
        CHECK(quantflow::state::exists("exists_test"));
    }

    TEST_CASE("returns false for non-existent key") {
        StateContext ctx;

        CHECK_FALSE(quantflow::state::exists("nonexistent"));
    }
}

TEST_SUITE("quantflow::state::remove") {
    TEST_CASE("deletes an existing key") {
        StateContext ctx;

        quantflow::state::set("to_delete", "value");
        CHECK(quantflow::state::exists("to_delete"));
        bool result = quantflow::state::remove("to_delete");
        CHECK(result);
        CHECK_FALSE(quantflow::state::exists("to_delete"));
    }

    TEST_CASE("returns false for non-existent key") {
        StateContext ctx;

        bool result = quantflow::state::remove("nonexistent");
        CHECK_FALSE(result);
    }
}

TEST_SUITE("quantflow::state::list") {
    TEST_CASE("lists all keys") {
        StateContext ctx;

        quantflow::state::set("key1", "value1");
        quantflow::state::set("key2", "value2");
        quantflow::state::set("key3", "value3");
        auto keys = quantflow::state::list();
        std::sort(keys.begin(), keys.end());
        CHECK(keys.size() == 3);
        CHECK(keys[0] == "key1");
        CHECK(keys[1] == "key2");
        CHECK(keys[2] == "key3");
    }

    TEST_CASE("returns empty vector when state is empty") {
        StateContext ctx;

        auto keys = quantflow::state::list();
        CHECK(keys.empty());
    }
}

TEST_SUITE("quantflow::state::clear") {
    TEST_CASE("removes all state") {
        StateContext ctx;

        quantflow::state::set("key1", "value1");
        quantflow::state::set("key2", "value2");
        CHECK(quantflow::state::list().size() == 2);
        quantflow::state::clear();
        CHECK(quantflow::state::list().size() == 0);
    }

    TEST_CASE("does not throw when state is already empty") {
        StateContext ctx;

        CHECK_NOTHROW(quantflow::state::clear());
        CHECK(quantflow::state::list().size() == 0);
    }
}

TEST_SUITE("quantflow::state key validation") {
    TEST_CASE("rejects empty key") {
        StateContext ctx;

        CHECK_THROWS_AS(quantflow::state::get(""), quantflow::state::InvalidKeyError);
    }

    TEST_CASE("rejects key with forward slash") {
        StateContext ctx;

        CHECK_THROWS_AS(quantflow::state::set("../escape", "evil"), quantflow::state::InvalidKeyError);
    }

    TEST_CASE("rejects key with backslash") {
        StateContext ctx;

        CHECK_THROWS_AS(quantflow::state::set("..\\escape", "evil"), quantflow::state::InvalidKeyError);
    }

    TEST_CASE("rejects key with double dots") {
        StateContext ctx;

        CHECK_THROWS_AS(quantflow::state::set("..", "evil"), quantflow::state::InvalidKeyError);
    }
}

TEST_SUITE("quantflow::state complex data") {
    TEST_CASE("handles deeply nested structures") {
        StateContext ctx;

        nlohmann::json complex_data = {
            {"portfolio", {
                {"holdings", nlohmann::json::array({
                    {{"symbol", "AAPL"}, {"quantity", 100}, {"price", 150.25}},
                    {{"symbol", "GOOGL"}, {"quantity", 50}, {"price", 2800.00}}
                })},
                {"total_value", 155025.0}
            }},
            {"signals", nlohmann::json::array({
                {{"type", "BUY"}, {"symbol", "AAPL"}, {"confidence", 0.85}},
                {{"type", "SELL"}, {"symbol", "TSLA"}, {"confidence", 0.72}}
            })},
            {"metadata", {
                {"last_update", "2024-01-15T10:30:00Z"},
                {"version", 2}
            }}
        };

        quantflow::state::set("complex", complex_data);
        auto retrieved = quantflow::state::get("complex");

        REQUIRE(retrieved.has_value());
        CHECK((*retrieved)["portfolio"]["holdings"].size() == 2);
        CHECK((*retrieved)["portfolio"]["holdings"][0]["symbol"] == "AAPL");
        CHECK((*retrieved)["signals"][1]["type"] == "SELL");
        CHECK((*retrieved)["metadata"]["version"] == 2);
    }

    TEST_CASE("overwrites existing key") {
        StateContext ctx;

        quantflow::state::set("key", "original");
        CHECK(quantflow::state::get("key").value() == "original");
        quantflow::state::set("key", "updated");
        CHECK(quantflow::state::get("key").value() == "updated");
    }
}
