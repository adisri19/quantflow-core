/**
 * Tests for quantflow::success(), quantflow::result() functions
 *
 * These functions write results to a file (result.json), not stdout.
 * Tests use ResultFileContext to set up a temp directory.
 */

#include <doctest/doctest.h>
#include <quantflow/quantflow.h>
#include "helpers/test_helpers.hpp"

using namespace quantflow_test;

TEST_SUITE("quantflow::success") {
    TEST_CASE("writes valid JSON with status success to result file") {
        ResultFileContext ctx;
        quantflow::success("Test message");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "success");
        CHECK(json["message"] == "Test message");
    }

    TEST_CASE("properly escapes special characters") {
        ResultFileContext ctx;
        quantflow::success("Message with \"quotes\" and \\backslash");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["message"] == "Message with \"quotes\" and \\backslash");
    }

    TEST_CASE("handles newlines and tabs") {
        ResultFileContext ctx;
        quantflow::success("Line1\nLine2\tTabbed");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["message"] == "Line1\nLine2\tTabbed");
    }

    TEST_CASE("handles unicode characters") {
        ResultFileContext ctx;
        quantflow::success("Hello World!");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["message"] == "Hello World!");
    }

    TEST_CASE("handles empty message") {
        ResultFileContext ctx;
        quantflow::success("");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "success");
        CHECK(json["message"] == "");
    }

    TEST_CASE("handles very long messages") {
        ResultFileContext ctx;
        std::string long_message(10000, 'x');
        quantflow::success(long_message);

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["message"].get<std::string>().size() == 10000);
    }
}

TEST_SUITE("quantflow::result with json") {
    TEST_CASE("writes custom JSON object to result file") {
        ResultFileContext ctx;
        quantflow::result({
            {"status", "success"},
            {"trade_id", "abc123"},
            {"amount", 100.5}
        });

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "success");
        CHECK(json["trade_id"] == "abc123");
        CHECK(json["amount"] == 100.5);
    }

    TEST_CASE("handles nested objects") {
        ResultFileContext ctx;
        quantflow::result({
            {"status", "success"},
            {"data", {
                {"price", 45000.0},
                {"volume", 1.5}
            }}
        });

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["data"]["price"] == 45000.0);
        CHECK(json["data"]["volume"] == 1.5);
    }

    TEST_CASE("handles arrays") {
        ResultFileContext ctx;
        quantflow::result({
            {"trades", {"trade1", "trade2", "trade3"}}
        });

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["trades"].is_array());
        CHECK(json["trades"].size() == 3);
    }

    TEST_CASE("handles empty object") {
        ResultFileContext ctx;
        quantflow::result(quantflow::json::object());

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json.is_object());
        CHECK(json.empty());
    }
}

TEST_SUITE("quantflow::result with status/message/data") {
    TEST_CASE("creates result with status and message") {
        ResultFileContext ctx;
        quantflow::result("success", "Trade completed");

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "success");
        CHECK(json["message"] == "Trade completed");
    }

    TEST_CASE("merges additional data fields") {
        ResultFileContext ctx;
        quantflow::result("success", "Trade executed", {
            {"trade_id", "12345"},
            {"price", 45000.50},
            {"quantity", 0.1}
        });

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "success");
        CHECK(json["message"] == "Trade executed");
        CHECK(json["trade_id"] == "12345");
        CHECK(json["price"] == 45000.50);
        CHECK(json["quantity"] == 0.1);
    }

    TEST_CASE("works with empty data") {
        ResultFileContext ctx;
        quantflow::result("error", "Something failed", {});

        std::string output = ctx.read_result();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["status"] == "error");
        CHECK(json["message"] == "Something failed");
        CHECK(json.size() == 2); // Only status and message
    }
}
