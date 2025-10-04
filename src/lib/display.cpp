#include "display.h"
#include "tools.h"
#include <iostream>
#include <sstream>

using namespace monitoring::tools;

namespace monitoring {
namespace display {

void most_hit_sections(std::stringstream &stream,
                       const window_metrics &metrics) {

  if (metrics.m_most_hit_sections.size() == 1) {
    stream << "Most hit section: " << metrics.m_most_hit_sections[0];
  } else {
    stream << "Most hit sections: ";
    for (const auto &section : metrics.m_most_hit_sections) {
      stream << section << " ";
    }
  }
  stream << " (" << metrics.m_most_hit_section_count;
  if (metrics.m_most_hit_section_count > 1)
    stream << " times";
  else
    stream << " time";
  stream << ")" << std::endl;
}

void count_per_operation(std::stringstream &stream,
                         const window_metrics &metrics) {
  stream << "Operations: ";
  http_verb verbs[] = {http_verb::Post, http_verb::Get, http_verb::Put,
                       http_verb::Patch, http_verb::Delete};
  for (auto verb : verbs) {
    int index = static_cast<int>(verb);
    if (metrics.m_operation_count[index]) {
      stream << to_string(verb) << "(" << metrics.m_operation_count[index]
             << ") ";
    }
  }
  stream << std::endl;
}

void success_rate(std::stringstream &stream, const window_metrics &metrics) {
  stream << "Success rate: " << std::round(metrics.m_success_rate) << "%"
         << std::endl
         << "Remote host with lowest success rate: "
         << metrics.m_lowest_success_rate_host << " ("
         << std::round(metrics.m_lowest_success_rate) << "%)" << std::endl;
}

void metrics(const window_metrics &metrics) {
  std::stringstream stream;

  stream << "-----" << std::endl
         << "Time window: " << local_display_time(metrics.m_start_date) << ", "
         << local_display_time(metrics.m_end_date) << ":" << std::endl;

  stream << "Number of hits: " << metrics.m_hit_nb << std::endl;

  if (metrics.m_hit_nb) {
    most_hit_sections(stream, metrics);
    count_per_operation(stream, metrics);
    success_rate(stream, metrics);
  }

  std::cout << stream.str();
}

void alert(const traffic_alert &alert) {
  std::stringstream stream;
  std::string end_period = local_display_time(alert.m_date);
  if (alert.m_alert) {
    stream << "*** High traffic generated an alert - hits = "
           << alert.m_hit_count_average << ", triggered at " << end_period
           << " ***";
  } else {
    stream << "*** Traffic back to normal at " << end_period << " ***";
  }
  stream << std::endl;

  std::cout << stream.str();
}
} // namespace display
} // namespace monitoring