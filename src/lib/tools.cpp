#include "tools.h"
#include <algorithm>
#include <time.h>

namespace monitoring {
namespace tools {

std::string extract_operation(const std::string &request) {
  auto space = request.find(' ', 0);
  std::string operation = "";
  if (std::string::npos != space) {
    operation = request.substr(0, space);
  }
  return operation;
}

std::optional<http_verb> get_http_verb(const std::string &httpRequest) {
  std::string verb = extract_operation(httpRequest);
  if ("POST" == verb) {
    return http_verb::Post;
  }
  if ("GET" == verb) {
    return http_verb::Get;
  }
  if ("PUT" == verb) {
    return http_verb::Put;
  }
  if ("PATCH" == verb) {
    return http_verb::Patch;
  }
  if ("DELETE" == verb) {
    return http_verb::Delete;
  }
  return std::nullopt;
}

std::string to_string(http_verb verb) {
  switch (verb) {
  case http_verb::Post:
    return "POST";
  case http_verb::Get:
    return "GET";
  case http_verb::Put:
    return "PUT";
  case http_verb::Patch:
    return "PATCH";
  case http_verb::Delete:
    return "DELETE";
  default:
    return "";
  }
}

int find_position(const std::vector<std::string> &data,
                  const std::string &value) {
  auto res = std::find(data.begin(), data.end(), value);
  if (res != data.end()) {
    return static_cast<int>(res - data.begin());
  }
  return -1;
}

std::string local_display_time(long long unix_timestamp) {
  time_t raw_time = static_cast<time_t>(unix_timestamp);
  struct tm date;
  localtime_s(&date, &raw_time);
  char display_date[50];
  asctime_s(display_date, sizeof display_date, &date);
  std::string result = display_date;
  result.pop_back();
  return result;
}
} // namespace tools
} // namespace monitoring