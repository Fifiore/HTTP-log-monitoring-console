#pragma once
#include "traffic_alert.h"
#include "window_metrics.h"

namespace monitoring {
namespace display {
void metrics(const window_metrics &metrics);
void alert(const traffic_alert &alert);
} // namespace display
} // namespace monitoring
