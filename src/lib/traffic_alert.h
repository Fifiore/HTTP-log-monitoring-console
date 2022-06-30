#pragma once
#include <string>

namespace monitoring {
  struct traffic_alert {
    long long m_date = 0;
    bool m_alert = false;
    int m_hit_count_average = 0;
    std::string m_msg = ";";
  };
}

