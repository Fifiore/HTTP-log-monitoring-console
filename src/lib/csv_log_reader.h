#pragma once
#include "traffic_log.h"
#include <fstream>
#include <optional>
#include <string>

namespace monitoring {
class csv_log_reader {
public:
  // Opens a file
  csv_log_reader(const std::string &file_path, char delimiter);

  // Fills a vector with values of current line, returns true if end of file
  // reached
  std::optional<traffic_log> get_line();

  csv_log_reader(const csv_log_reader &) = delete;
  csv_log_reader &operator=(const csv_log_reader &) = delete;

private:
  std::ifstream m_file;
  std::istream *m_stream = nullptr;
  char m_delimiter = 0;
  bool m_first_row = true;

  struct log_format {
    int m_date_index = -1;
    int m_request_index = -1;
    int m_status_index = -1;
    int m_remote_host_index = 1;
  };
  log_format m_log_format;

  bool read_header();
  void save_column_index(const std::string &value, int index);
  void fill_log(traffic_log &log, const std::string &value, int value_index);
  ;
};
} // namespace monitoring
