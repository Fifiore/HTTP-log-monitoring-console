#include <gtest/gtest.h>
#include "Tools.h"

using namespace monitoring::tools;

TEST(tools_test, get_http_verb) {
  EXPECT_EQ(http_verb::Delete, get_http_verb("DELETE /api/help HTTP/1.0"));
  EXPECT_EQ(http_verb::Get, get_http_verb("GET /api/help HTTP/1.0"));
  EXPECT_EQ(http_verb::Patch, get_http_verb("PATCH /api/help HTTP/1.0"));
  EXPECT_EQ(http_verb::Post, get_http_verb("POST /api/help HTTP/1.0"));
  EXPECT_EQ(http_verb::Put, get_http_verb("PUT /api/help HTTP/1.0"));
  EXPECT_EQ(std::nullopt, get_http_verb("ALLO /api/help HTTP/1.0"));
}

TEST(tools_test, to_string_http_verb) {
  EXPECT_EQ("DELETE", to_string(http_verb::Delete));
  EXPECT_EQ("GET", to_string(http_verb::Get));
  EXPECT_EQ("PATCH", to_string(http_verb::Patch));
  EXPECT_EQ("POST", to_string(http_verb::Post));
  EXPECT_EQ("PUT", to_string(http_verb::Put));
}

TEST(tools_test, store_count_in_map) {
  std::map<std::string, int> count;

  increment_count(count, std::string("a"));
  increment_count(count, std::string("c"));
  increment_count(count, std::string("a"));
  increment_count(count, std::string("b"));

  EXPECT_EQ(3, static_cast<int>(count.size()));
  EXPECT_EQ(2, count["a"]);
  EXPECT_EQ(1, count["b"]);
  EXPECT_EQ(1, count["c"]);
}

TEST(tools_test, find_position) {
  std::vector<std::string> data = { "a", "b", "c", "d" };

  EXPECT_EQ(2, find_position(data, "c"));
  EXPECT_EQ(-1, find_position(data, "g"));
}