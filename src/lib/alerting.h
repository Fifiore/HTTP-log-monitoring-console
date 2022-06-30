#pragma once
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "traffic_log.h"
#include "traffic_alert.h"
#include "channel.h"

namespace monitoring {
  class alerting {
  public:

    alerting(int window_size, int wait_delay) :
      m_window_size(window_size), m_wait_delay(wait_delay) {}

    //Overwrites the default threshold (10 logs/second)
    void set_traffic_alert_threshold(int traffic_alert_threshold);

    //Sends a log to the alerting and triggers analysis
    void push_log(const traffic_log& log);

    //To associate an output channel
    void set_output_channel(channel<traffic_alert>* pipe);

    alerting() = delete;
    alerting(const alerting&) = delete;
    alerting& operator=(const alerting&) = delete;

  private:
    int m_window_size = 0;
    int m_wait_delay = 0;
    int m_threshold = 10;
    long long m_window_start_time = 0;
    bool m_alert = false;
    int m_cumulated_hit_count = 0;
    bool m_first_window_computed = false;

    //Map + most recent date for the moving time window
    long long m_most_recent_date = 0;
    std::map<long long, int> m_hit_count;

    channel<traffic_alert>* m_alert_pipe = nullptr;

    void compute();
    bool complete_period_stored();
    void compute_traffic_alert();
    void remove_old_logs();
    void traffic_alert_status_changed(int hit_count_average, long long window_end_date);
    void emit_alert(traffic_alert alert);
  };

}

