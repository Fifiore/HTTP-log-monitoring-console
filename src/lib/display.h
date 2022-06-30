#pragma once
#include "window_metrics.h"
#include "traffic_alert.h"

namespace monitoring {
  namespace display {
    void metrics(const window_metrics& metrics);
    void alert(const traffic_alert& alert);
  }
}
