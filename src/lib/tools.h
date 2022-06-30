#pragma once
#include <string>
#include <unordered_map>
#include <optional>
#include <map>

namespace monitoring {
  namespace tools {

    enum class http_verb { Post, Get, Put, Patch, Delete };

    //Return the httpVerb of a request, nullopt if invalid
    std::optional<http_verb> get_http_verb(const std::string& http_request);

    //Convert an http_verb into a readable string
    std::string to_string(http_verb verb);

    //Add a occurence in a unordered_map to count keys
    template <typename Map, typename T>
    void increment_count(Map& map_count, T key) {
      auto it = map_count.find(key);
      if (map_count.end() != it) {
        it->second++;
      }
      else {
        map_count[key] = 1;
      }
    }

    //Return the position of the value if exists, -1 otherwise
    int find_position(const std::vector<std::string>& data, const std::string& value);

    //Return a human readable format of a Unix timestamp (local time)
    std::string local_display_time(long long unix_timestamp);

  }
}
