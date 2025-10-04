#include "monitoring.h"
#include "alerting.h"
#include "channel.h"
#include "csv_log_reader.h"
#include "display.h"
#include "metrics_tracking.h"
#include "traffic_log.h"
#include <iostream>

namespace monitoring {

inline const char CSV_DELIMITER = ',';

// Logs could arrive with a delay.
// Wait {WAIT_DELAY} seconds before considering a date for analysis
inline const int WAIT_DELAY = 4;

inline const int METRIC_WINDOW = 10;         // seconds
inline const int TRAFFIC_ALERT_WINDOW = 120; // seconds

// Worker to read the input log pipe and send it to metrics tracking and
// alerting
void log_worker(metrics_tracking &metrics_tracking, alerting &alerting,
                channel<traffic_log> &pipe) {
  while (true) {
    auto data = pipe.receive();
    if (!data) {
      return; // pipe is closed
    }
    try {
      metrics_tracking.push_log(data.value());
      alerting.push_log(data.value());
    } catch (const std::exception &ex) {
      std::cout << ex.what() << std::endl;
      break;
    }
  }
}

// Worker to read metrics pipe and display readable messages
void display_metrics_worker(channel<window_metrics> &pipe) {
  while (true) {
    auto metrics = pipe.receive();
    if (!metrics) {
      return; // pipe is closed
    }
    try {
      display::metrics(metrics.value());
    } catch (const std::exception &ex) {
      std::cout << ex.what() << std::endl;
      break;
    }
  }
}

// Worker to read alerts pipe and display readable messages
void display_alert_worker(channel<traffic_alert> &pipe) {
  while (true) {
    auto alert = pipe.receive();
    if (!alert) {
      return; // pipe is closed
    }
    try {
      display::alert(alert.value());
    } catch (const std::exception &ex) {
      std::cout << ex.what() << std::endl;
      break;
    }
  }
}

void read_and_compute_logs(csv_log_reader &reader,
                           metrics_tracking &metrics_tracking,
                           alerting &alerting) {

  // Initate communications channels and associate it to metrics and alerts
  // Launch workers

  channel<window_metrics> output_metrics_pipe;
  std::thread output_metrics_thread(
      [&]() { display_metrics_worker(output_metrics_pipe); });
  metrics_tracking.set_output_channel(&output_metrics_pipe);

  channel<traffic_alert> output_alert_pipe;
  std::thread output_alert_thread(
      [&]() { display_alert_worker(output_alert_pipe); });
  alerting.set_output_channel(&output_alert_pipe);

  channel<traffic_log> log_pipe;
  std::thread log_worker_thread(
      [&]() { log_worker(metrics_tracking, alerting, log_pipe); });

  // Read logs from stream (file or strandard input)
  while (auto log = reader.get_line()) {
    log_pipe.send(log.value());
  }

  // All logs have been sent
  // Closing and waiting for workers

  log_pipe.close();
  log_worker_thread.join();

  output_alert_pipe.close();
  output_alert_thread.join();

  output_metrics_pipe.close();
  output_metrics_thread.join();
}

void execute(const std::string &input_file, int threshold) {
  metrics_tracking metrics_tracking(METRIC_WINDOW, WAIT_DELAY);
  alerting alerting(TRAFFIC_ALERT_WINDOW, WAIT_DELAY);

  if (threshold) {
    alerting.set_traffic_alert_threshold(threshold);
  }

  try {
    csv_log_reader reader(input_file, CSV_DELIMITER);
    read_and_compute_logs(reader, metrics_tracking, alerting);
  } catch (const std::exception &ex) {
    std::cout << ex.what() << std::endl;
  }
}
} // namespace monitoring