#include "alerting.h"
#include "tools.h"
#include <algorithm>

using namespace monitoring::tools;

namespace monitoring {

void alerting::set_output_channel(channel<traffic_alert> *pipe) {
  m_alert_pipe = pipe;
}

void alerting::set_traffic_alert_threshold(int traffic_alert_threshold) {
  m_threshold = traffic_alert_threshold;
}

void alerting::push_log(const traffic_log &log) {
  if (0 == log.m_date) {
    // invalid logs are ignored
    return;
  }
  if (m_most_recent_date < log.m_date) {
    m_most_recent_date = log.m_date;
  }
  increment_count(m_hit_count, log.m_date);
  compute();
}

bool alerting::complete_period_stored() {
  // Wait a delay before considering the first date
  if (!m_window_start_time &&
      m_most_recent_date - m_hit_count.begin()->first >= m_wait_delay) {
    m_window_start_time = m_hit_count.begin()->first;
  }
  // We want the complete period plus a delay to compute
  long long last_window_end_date = m_window_start_time + m_window_size - 1;
  return m_window_start_time
             ? m_most_recent_date - last_window_end_date >= m_wait_delay
             : false;
}

void alerting::emit_alert(traffic_alert alert) {
  if (m_alert_pipe) {
    m_alert_pipe->send(std::move(alert));
  }
}

void alerting::traffic_alert_status_changed(int hit_count_average,
                                            long long window_end_date) {
  m_alert = !m_alert;
  traffic_alert alert;
  alert.m_alert = m_alert;
  alert.m_hit_count_average = hit_count_average;
  alert.m_date = window_end_date;
  emit_alert(std::move(alert));
}

void alerting::compute_traffic_alert() {
  while (complete_period_stored()) {

    // First and last date in the window
    long long start_date = m_window_start_time;
    long long end_date = m_window_start_time + m_window_size - 1;

    if (!m_first_window_computed) {
      // First window: cumulate hits of every date received in the window
      auto lower_bound = m_hit_count.lower_bound(start_date);
      auto upper_bound = m_hit_count.upper_bound(end_date);
      std::for_each(lower_bound, upper_bound, [&](const auto &hits) {
        m_cumulated_hit_count += hits.second;
      });
      m_first_window_computed = true;
    } else {
      // Remove hit counts for the date exiting the window (1 before the window)
      auto exiting_date = m_hit_count.find(start_date - 1);
      if (m_hit_count.end() != exiting_date) {
        m_cumulated_hit_count -= exiting_date->second;
      }
      // Add hit counts for date entering the window
      auto entering_date = m_hit_count.find(end_date);
      if (m_hit_count.end() != entering_date) {
        m_cumulated_hit_count += entering_date->second;
      }
    }
    int hit_count_average =
        static_cast<int>(m_cumulated_hit_count / m_window_size);
    if (!m_alert && hit_count_average >= m_threshold) {
      traffic_alert_status_changed(hit_count_average, end_date);
    } else if (m_alert && hit_count_average < m_threshold) {
      traffic_alert_status_changed(hit_count_average, end_date);
    }
    // Slide the window of 1 sc
    m_window_start_time++;
  }
}

void alerting::remove_old_logs() {
  if (m_hit_count.empty()) {
    return;
  }
  long long date = m_hit_count.begin()->first;

  while (date < m_window_start_time - 1) {
    m_hit_count.erase(date);
    date++;
  }
}

void alerting::compute() {
  if (m_hit_count.empty()) {
    return;
  }
  compute_traffic_alert();
  remove_old_logs();
}

} // namespace monitoring