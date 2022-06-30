#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace monitoring {
  struct window_metrics {
    //first date included
    long long m_start_date = 0;
    //last date included
    long long m_end_date = 0;
    int m_hit_nb = 0;
    std::vector<std::string> m_most_hit_sections;
    int m_most_hit_section_count = 0;
    int m_operation_count[5] = { };
    double m_success_rate;
    std::string m_lowest_success_rate_host = "";
    double m_lowest_success_rate = 0;
  };
}

