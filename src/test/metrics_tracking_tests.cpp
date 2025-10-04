#include "metrics_tracking.h"
#include "tools.h"
#include "traffic_log.h"
#include "window_metrics.h"
#include <gtest/gtest.h>
#include <string>

namespace monitoring {
std::string extract_section(const std::string &request);
}

using namespace monitoring;
using namespace monitoring::tools;

TEST(metrics_tracking_test, extract_section_invalid_request) {
  std::string s1;
  std::string s2 = "  hello";
  EXPECT_EQ("", extract_section(s1));
  EXPECT_EQ("", extract_section(s2));
}

TEST(metrics_tracking_test, extract_section_valid_request) {
  std::string s1 = "GET /api/user HTTP/1.0";
  std::string s2 = "POST /report HTTP/1.0";
  EXPECT_EQ("/api", extract_section(s1));
  EXPECT_EQ("/report", extract_section(s2));
}

std::vector<traffic_log>
build_logs(std::vector<std::vector<std::string>> inputs) {
  std::vector<traffic_log> logs;
  for (const auto &input : inputs) {
    if (4 == input.size()) {
      traffic_log log;
      log.m_date = std::stoll(input[0]);
      log.m_status = std::stoi(input[1]);
      log.m_request = input[2];
      log.m_remote_host = input[3];
      logs.push_back(log);
    }
  }
  return logs;
}

std::vector<std::vector<std::string>> input_logs = {
    {"2", "200", "PUT /api/user HTTP/1.0", "10.0.0.3"},
    {"1", "200", "DELETE /api/user HTTP/1.0", "10.0.0.1"},
    {"1", "404", "GET /api/user HTTP/1.0", "10.0.0.2"},
    {"1", "200", "POST /report HTTP/1.0", "10.0.0.1"},
    {"2", "500", "PUT /api/user HTTP/1.0", "10.0.0.1"},
    {"3", "200", "PATCH /report HTTP/1.0", "10.0.0.1"},
    {"3", "200", "GET /api/user HTTP/1.0", "10.0.0.1"},
    {"4", "200", "GET /api/user HTTP/1.0", "10.0.0.2"},
    {"4", "200", "PUT /api/user HTTP/1.0", "10.0.0.3"},
    {"5", "200", "POST /report HTTP/1.0", "10.0.0.1"},
    {"5", "404", "GET /report HTTP/1.0", "10.0.0.4"},
    {"7", "500", "PUT /api/user HTTP/1.0", "10.0.0.1"},
    {"7", "200", "PUT /report HTTP/1.0", "10.0.0.1"},
    {"9", "200", "GET /report HTTP/1.0", "10.0.0.1"},
    {"10", "404", "GET /api/user HTTP/1.0", "10.0.0.2"},
    {"10", "200", "PUT /api/user HTTP/1.0", "10.0.0.3"},
    {"11", "200", "POST /report HTTP/1.0", "10.0.0.1"},
    {"11", "500", "PUT /api/user HTTP/1.0", "10.0.0.1"},
    {"12", "200", "PUT /report HTTP/1.0", "10.0.0.1"}};

TEST(metrics_tracking_test, execution) {
  channel<window_metrics> output_pipe;
  metrics_tracking tracking(3, 1);
  tracking.set_output_channel(&output_pipe);

  std::vector<traffic_log> logs = build_logs(input_logs);
  for (const auto &log : logs) {
    tracking.push_log(log);
  }

  output_pipe.close();

  std::vector<window_metrics> alerts;
  while (auto output = output_pipe.receive()) {
    alerts.push_back(output.value());
  }

  EXPECT_EQ(3, static_cast<int>(alerts.size()));

  EXPECT_EQ(1, alerts[0].m_start_date);
  EXPECT_EQ(3, alerts[0].m_end_date);

  EXPECT_EQ(4, alerts[1].m_start_date);
  EXPECT_EQ(6, alerts[1].m_end_date);

  EXPECT_EQ(7, alerts[2].m_start_date);
  EXPECT_EQ(9, alerts[2].m_end_date);

  // The window [10,12] is ignored due to 1 second delay needed to accept the
  // date 12

  EXPECT_EQ(7, alerts[0].m_hit_nb);
  EXPECT_EQ(4, alerts[1].m_hit_nb);
  EXPECT_EQ(3, alerts[2].m_hit_nb);

  EXPECT_EQ(1, static_cast<int>(alerts[0].m_most_hit_sections.size()));
  EXPECT_EQ("/api", alerts[0].m_most_hit_sections[0]);
  EXPECT_EQ(5, alerts[0].m_most_hit_section_count);

  EXPECT_EQ(2, static_cast<int>(alerts[1].m_most_hit_sections.size()));
  EXPECT_EQ("/api", alerts[1].m_most_hit_sections[0]);
  EXPECT_EQ("/report", alerts[1].m_most_hit_sections[1]);
  EXPECT_EQ(2, alerts[1].m_most_hit_section_count);

  EXPECT_EQ(1, static_cast<int>(alerts[2].m_most_hit_sections.size()));
  EXPECT_EQ("/report", alerts[2].m_most_hit_sections[0]);
  EXPECT_EQ(2, alerts[2].m_most_hit_section_count);

  EXPECT_EQ(1,
            alerts[0].m_operation_count[static_cast<int>(http_verb::Delete)]);
  EXPECT_EQ(2, alerts[0].m_operation_count[static_cast<int>(http_verb::Get)]);
  EXPECT_EQ(1, alerts[0].m_operation_count[static_cast<int>(http_verb::Patch)]);
  EXPECT_EQ(1, alerts[0].m_operation_count[static_cast<int>(http_verb::Post)]);
  EXPECT_EQ(2, alerts[0].m_operation_count[static_cast<int>(http_verb::Put)]);

  EXPECT_TRUE(abs(71.41 - alerts[0].m_success_rate) < 0.1);
  EXPECT_TRUE(abs(75 - alerts[1].m_success_rate) < 0.1);
  EXPECT_TRUE(abs(66.66 - alerts[2].m_success_rate) < 0.1);

  EXPECT_TRUE(abs(0 - alerts[0].m_lowest_success_rate) < 0.1);
  EXPECT_TRUE(abs(0 - alerts[1].m_lowest_success_rate) < 0.1);
  EXPECT_TRUE(abs(66.66 - alerts[2].m_lowest_success_rate) < 0.1);

  EXPECT_EQ("10.0.0.2", alerts[0].m_lowest_success_rate_host);
  EXPECT_EQ("10.0.0.4", alerts[1].m_lowest_success_rate_host);
  EXPECT_EQ("10.0.0.1", alerts[2].m_lowest_success_rate_host);
}
