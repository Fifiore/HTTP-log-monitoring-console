#include "metrics_tracking.h"
#include "tools.h"
#include <algorithm>

#include <sstream>

using namespace monitoring::tools;

namespace monitoring {

  void metrics_tracking::set_output_channel(channel<window_metrics>* pipe) {
    m_output_pipe = pipe;
  }

  std::string extract_section(const std::string& request) {
    std::string section = "";
    std::istringstream request_stream(request);
    std::string value;
    std::vector<std::string> elements;
    while (std::getline(request_stream, value, ' ')) {
      elements.push_back(value);
    }
    if (elements.size() == 3) {
      auto slash = elements[1].find('/', 1);
      if (std::string::npos == slash) {
        section = elements[1];
      }
      else {
        section = elements[1].substr(0, slash);
      }
    }
    return section;
  }

  void metrics_tracking::store_log(traffic_log log) {
    auto it = m_logs.find(log.m_date);
    if (m_logs.end() != it) {
      it->second.push_back(std::move(log));
    }
    else {
      m_logs[log.m_date] = { log };
    }
  }

  void metrics_tracking::push_log(const traffic_log& log) {
    if (0 == log.m_date) {
      //invalid logs are ignored
      return;
    }
    if (m_most_recent_date < log.m_date) {
      m_most_recent_date = log.m_date;
    }
    store_log(log);
    compute();
  }

  void compute_most_hit_sections(const std::unordered_map<std::string, int>& hit_per_section, window_metrics& metrics) {
    
    int highest_count = 0;

    for (const auto& stat : hit_per_section) {
      metrics.m_hit_nb += stat.second;
      if (highest_count < stat.second) {
        highest_count = stat.second;
        metrics.m_most_hit_sections.clear();
        metrics.m_most_hit_sections.push_back(stat.first);
      }
      else if (highest_count == stat.second) {
        metrics.m_most_hit_sections.push_back(stat.first);
      }
    }
    metrics.m_most_hit_section_count = highest_count;
  }

  void metrics_tracking::increment_status_count(std::unordered_map<std::string_view, status_count>& stats, std::string_view host, bool success) {
    auto it = stats.find(host);
    if (stats.end() != it) {
      if (success)
        it->second.m_success_count++;
      else
        it->second.m_failure_count++;
    }
    else {
      status_count status;
      if (success)
        status.m_success_count = 1;
      else
        status.m_failure_count = 1;
      stats[host] = status;
    }
  }

  void metrics_tracking::compute_success_rate(const std::unordered_map<std::string_view, status_count>& stats_per_remote_host, window_metrics& metrics) {
    int success_count = 0;
    int request_count = 0;
    std::string lowest_success_rate_host = "";
    double lowest_success_rate = 100;
    for (const auto& hostStat : stats_per_remote_host) {
      int host_request_count = hostStat.second.m_success_count + hostStat.second.m_failure_count;
      double host_success_rate = hostStat.second.m_success_count * 100 / static_cast<double>(host_request_count);
      if (host_success_rate < lowest_success_rate) {
        lowest_success_rate = host_success_rate;
        lowest_success_rate_host = hostStat.first;
      }
      request_count += host_request_count;
      success_count += hostStat.second.m_success_count;
    }
    if (request_count) {
      double success_rate = success_count * 100 / static_cast<double>(request_count);
      metrics.m_success_rate = success_rate;
      metrics.m_lowest_success_rate_host = lowest_success_rate_host;
      metrics.m_lowest_success_rate = lowest_success_rate;
    }
  }

  bool metrics_tracking::complete_period_stored() {
    //Wait a delay before considering the first date
    if (!m_window_start_time && m_most_recent_date - m_logs.begin()->first >= m_wait_delay) {
      m_window_start_time = m_logs.begin()->first;
    }
    //We want the complete period plus a delay to compute
    long long last_window_end_date = m_window_start_time + m_window_size - 1;
    return m_window_start_time ? m_most_recent_date - last_window_end_date >= m_wait_delay : false;
  }

  void increment_http_operations_count(window_metrics& metric, const std::string& request) {
    auto verb = get_http_verb(request);
    if (verb) {
      metric.m_operation_count[static_cast<int>(verb.value())]++;
    }
  }

  void metrics_tracking::compute_metrics() {
    while (complete_period_stored()) {
      window_metrics metrics;
      //First and last date in the window
      metrics.m_start_date = m_window_start_time;
      metrics.m_end_date = m_window_start_time + m_window_size - 1;

      //To aggregate hits per section
      std::unordered_map<std::string, int> hit_per_section;

      //To aggregate successes/failures by remote host
      std::unordered_map<std::string_view, status_count> stats_per_remote_host;

      auto start_bound = m_logs.lower_bound(metrics.m_start_date);
      auto end_bound = m_logs.upper_bound(metrics.m_end_date);

      //For every date received in the window
      std::for_each(start_bound, end_bound, [&](const auto& date_entries) {
        for (const auto& log : date_entries.second) {
          increment_count(hit_per_section, extract_section(log.m_request));
          //Aggregate hits per HTTP verb (PUT,GET...)
          increment_http_operations_count(metrics, log.m_request);
          if (log.m_status > 0) {
            increment_status_count(stats_per_remote_host, log.m_remote_host, log.m_status < 300);
          }
        }
        });

      //Fill metrics
      compute_most_hit_sections(hit_per_section, metrics);
      compute_success_rate(stats_per_remote_host, metrics);
      //Push in output pipe
      emit_metrics(std::move(metrics));
      m_window_start_time += m_window_size;
    }
  }

  void metrics_tracking::emit_metrics(window_metrics metrics) {
    if (m_output_pipe) {
      m_output_pipe->send(std::move(metrics));
    }
  }


  void metrics_tracking::remove_old_logs() {
    if (m_logs.empty()) {
      return;
    }
    long long date = m_logs.begin()->first;

    while (date < m_window_start_time) {
      m_logs.erase(date);
      date++;
    }
  }

  void metrics_tracking::compute() {
    if (m_logs.empty()) {
      return;
    }
    compute_metrics();
    remove_old_logs();
  }
  
}