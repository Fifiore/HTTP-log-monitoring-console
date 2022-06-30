#pragma once
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "traffic_log.h"
#include "window_metrics.h"
#include "channel.h"

namespace monitoring {
  class metrics_tracking {
  public:

    metrics_tracking(int window_size, int wait_delay) :
      m_window_size(window_size), m_wait_delay(wait_delay) {}

    //Sends a log to the metrics tracking and triggers analysis
    void push_log(const traffic_log& log);

    //To associate an output channel
    void set_output_channel(channel<window_metrics>* pipe);

    metrics_tracking() = delete;
    metrics_tracking(const metrics_tracking&) = delete;
    metrics_tracking& operator=(const metrics_tracking&) = delete;

  private:
    int m_window_size = 0;
    int m_wait_delay = 0;
    long long m_window_start_time = 0;

    //Map + most recent date for the moving time window
    long long m_most_recent_date = 0;
    std::map<long long, std::vector<traffic_log>> m_logs;

    channel<window_metrics>* m_output_pipe = nullptr;

    void compute();
    bool complete_period_stored();
    void store_log(traffic_log log);
    void compute_metrics();
    void remove_old_logs();
    void emit_metrics(window_metrics metrics);
    struct status_count {
      int m_success_count = 0;
      int m_failure_count = 0;
    };
    void compute_success_rate(const std::unordered_map<std::string_view, status_count>& stats_per_remote_host, window_metrics& metrics);
    void increment_status_count(std::unordered_map<std::string_view, status_count>& stats, std::string_view host, bool success);
  };

}

