cmake_minimum_required(VERSION 3.15)
project(log_monitoring)

find_package(GTest)

add_library(log_monitoring_lib STATIC
	src/lib/alerting.cpp 
	src/lib/alerting.h 
	src/lib/channel.h 
	src/lib/csv_log_reader.cpp 
	src/lib/csv_log_reader.h 
	src/lib/display.cpp 
	src/lib/display.h 
	src/lib/metrics_tracking.cpp 
	src/lib/metrics_tracking.h 
	src/lib/monitoring.cpp 
	src/lib/monitoring.h 
	src/lib/tools.cpp 
	src/lib/tools.h 
	src/lib/traffic_alert.h
	src/lib/traffic_log.h 
	src/lib/window_metrics.h)
target_compile_features(log_monitoring_lib PRIVATE cxx_std_17)

add_executable(log_monitoring src/log_monitoring.cpp)
target_compile_features(log_monitoring PRIVATE cxx_std_17)
target_include_directories(log_monitoring PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)

add_executable(log_monitoring_test src/test/alerting_test.cpp src/test/metrics_tracking_tests.cpp src/test/tools_test.cpp)
target_compile_features(log_monitoring_test PRIVATE cxx_std_17)
target_include_directories(log_monitoring_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)

target_link_libraries(log_monitoring PRIVATE log_monitoring_lib)
target_link_libraries(log_monitoring_test PRIVATE GTest::gtest GTest::gtest_main log_monitoring_lib)

