/**
 * Tests for quantflow::metric() and quantflow::log() functions
 *
 * These functions write to stdout (not to result file).
 */

#include <doctest/doctest.h>
#include <quantflow/quantflow.h>
#include "helpers/test_helpers.hpp"

using namespace quantflow_test;

TEST_SUITE("quantflow::metric") {
    TEST_CASE("outputs JSON with _metric field to stdout") {
        CaptureStdout capture;
        quantflow::metric("price", {{"symbol", "BTC/USD"}, {"value", 45000.0}});

        std::string output = capture.get();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["_metric"] == "price");
        CHECK(json["symbol"] == "BTC/USD");
        CHECK(json["value"] == 45000.0);
    }

    TEST_CASE("includes timestamp field") {
        CaptureStdout capture;
        quantflow::metric("signal", {{"type", "BUY"}});

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json.contains("timestamp"));
        CHECK(json["timestamp"].is_number());
        // Timestamp should be a reasonable Unix milliseconds value
        long long ts = json["timestamp"].get<long long>();
        CHECK(ts > 1700000000000);  // After Nov 2023
    }

    TEST_CASE("handles empty data") {
        CaptureStdout capture;
        quantflow::metric("heartbeat", {});

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["_metric"] == "heartbeat");
        CHECK(json.contains("timestamp"));
    }

    TEST_CASE("handles nested data") {
        CaptureStdout capture;
        quantflow::metric("trade", {
            {"order", {
                {"id", "12345"},
                {"side", "buy"},
                {"quantity", 1.5}
            }}
        });

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["_metric"] == "trade");
        CHECK(json["order"]["id"] == "12345");
        CHECK(json["order"]["side"] == "buy");
        CHECK(json["order"]["quantity"] == 1.5);
    }

    TEST_CASE("handles array data") {
        CaptureStdout capture;
        quantflow::metric("prices", {{"values", {100.0, 101.5, 99.8}}});

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["values"].is_array());
        CHECK(json["values"].size() == 3);
        CHECK(json["values"][0] == 100.0);
    }

    TEST_CASE("outputs newline at end") {
        CaptureStdout capture;
        quantflow::metric("test", {});

        std::string output = capture.get();
        CHECK(output.back() == '\n');
    }
}

TEST_SUITE("quantflow::log") {
    TEST_CASE("outputs JSON with message field to stderr") {
        CaptureStderr capture;
        quantflow::log("Starting bot execution");

        std::string output = capture.get();
        REQUIRE(!output.empty());

        auto json = quantflow::json::parse(output);
        CHECK(json["message"] == "Starting bot execution");
    }

    TEST_CASE("defaults to info level") {
        CaptureStderr capture;
        quantflow::log("Test message");

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["level"] == "info");
    }

    TEST_CASE("includes timestamp") {
        CaptureStderr capture;
        quantflow::log("Test message");

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json.contains("timestamp"));
        CHECK(json["timestamp"].is_number());
        // Timestamp should be a reasonable Unix milliseconds value
        long long ts = json["timestamp"].get<long long>();
        CHECK(ts > 1700000000000);  // After Nov 2023
    }

    TEST_CASE("supports warn level") {
        CaptureStderr capture;
        quantflow::log("Warning message", {}, quantflow::LogLevel::Warn);

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["level"] == "warn");
        CHECK(json["message"] == "Warning message");
    }

    TEST_CASE("supports error level") {
        CaptureStderr capture;
        quantflow::log("Error message", {}, quantflow::LogLevel::Error);

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["level"] == "error");
        CHECK(json["message"] == "Error message");
    }

    TEST_CASE("merges data fields") {
        CaptureStderr capture;
        quantflow::log("Order placed", {{"order_id", "12345"}, {"symbol", "BTC"}});

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["message"] == "Order placed");
        CHECK(json["order_id"] == "12345");
        CHECK(json["symbol"] == "BTC");
    }

    TEST_CASE("supports data and level together") {
        CaptureStderr capture;
        quantflow::log("Order failed", {{"order_id", "12345"}}, quantflow::LogLevel::Error);

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["level"] == "error");
        CHECK(json["message"] == "Order failed");
        CHECK(json["order_id"] == "12345");
    }

    TEST_CASE("properly escapes special characters in message") {
        CaptureStderr capture;
        quantflow::log("Error: \"file not found\"");

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["message"] == "Error: \"file not found\"");
    }

    TEST_CASE("handles empty message") {
        CaptureStderr capture;
        quantflow::log("");

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["message"] == "");
    }

    TEST_CASE("handles newlines in message") {
        CaptureStderr capture;
        quantflow::log("Line 1\nLine 2");

        std::string output = capture.get();
        auto json = quantflow::json::parse(output);

        CHECK(json["message"] == "Line 1\nLine 2");
    }

    TEST_CASE("outputs newline at end") {
        CaptureStderr capture;
        quantflow::log("test");

        std::string output = capture.get();
        CHECK(output.back() == '\n');
    }
}
