#pragma once
#include <string>

namespace monitoring {
  struct traffic_log {
    long long m_date = 0;
    int m_status = 0;
    std::string m_request = "";
    std::string m_remote_host = "";
  };
}