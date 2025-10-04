#include "alerting.h"
#include "channel.h"
#include "traffic_alert.h"
#include "traffic_log.h"
#include <gtest/gtest.h>

using namespace monitoring;

std::vector<traffic_log> buildLogs(std::vector<long long> dates) {
  std::vector<traffic_log> logs;
  for (long long date : dates) {
    traffic_log log;
    log.m_date = date;
    logs.push_back(log);
  }
  return logs;
}

TEST(alerting_test, execution) {
  channel<traffic_alert> output_pipe;
  alerting alerting(3, 2);
  alerting.set_traffic_alert_threshold(2);
  alerting.set_output_channel(&output_pipe);

  std::vector<traffic_log> logs =
      buildLogs({2,  1, 2, 2, 3,  3,  2,  2,  4,  3,  5,  5,  6,  5,  8, 9,
                 10, 9, 9, 9, 10, 10, 12, 11, 11, 11, 11, 15, 17, 16, 19});
  for (const auto &log : logs) {
    alerting.push_log(log);
  }
  output_pipe.close();

  std::vector<traffic_alert> alerts;
  while (auto output = output_pipe.receive()) {
    alerts.push_back(output.value());
  }
  EXPECT_EQ(4, static_cast<int>(alerts.size()));

  EXPECT_EQ(3, alerts[0].m_date);
  EXPECT_EQ(true, alerts[0].m_alert);

  EXPECT_EQ(6, alerts[1].m_date);
  EXPECT_EQ(false, alerts[1].m_alert);

  EXPECT_EQ(10, alerts[2].m_date);
  EXPECT_EQ(true, alerts[2].m_alert);
  EXPECT_EQ(13, alerts[3].m_date);
  EXPECT_EQ(false, alerts[3].m_alert);
}
