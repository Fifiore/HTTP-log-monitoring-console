#include "csv_log_reader.h"
#include <iostream>
#include <sstream>
namespace monitoring {

csv_log_reader::csv_log_reader(const std::string &path, char delimiter)
    : m_delimiter(delimiter) {
  if ("-" == path) {
    m_stream = &std::cin;
  } else {
    m_file.open(path);
    if (m_file.fail()) {
      throw std::invalid_argument("File \"" + path + "\" does not exist");
    }
    m_stream = &m_file;
  }
}

void csv_log_reader::save_column_index(const std::string &value, int index) {
  if ("date" == value) {
    m_log_format.m_date_index = index;
  } else if ("request" == value) {
    m_log_format.m_request_index = index;
  } else if ("status" == value) {
    m_log_format.m_status_index = index;
  } else if ("remotehost" == value) {
    m_log_format.m_remote_host_index = index;
  }
}

void csv_log_reader::fill_log(traffic_log &log, const std::string &value,
                              int value_index) {

  if (m_log_format.m_date_index == value_index) {
    log.m_date = std::stoll(value);
  } else if (m_log_format.m_request_index == value_index) {
    log.m_request = value;
  } else if (m_log_format.m_status_index == value_index) {
    log.m_status = std::stoi(value);
  } else if (m_log_format.m_remote_host_index == value_index) {
    log.m_remote_host = value;
  }
}

void remove_quotes(std::string &value) {
  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    value = value.substr(1, value.size() - 2);
  }
}

template <typename Func>
bool for_each_value_of_next_line(std::istream *stream, char delimiter,
                                 Func function) {
  if (!stream)
    return true;

  std::string row;
  bool get_line_succeeded = false;
  if (std::getline(*stream, row)) {
    std::istringstream row_stream(row);
    std::string value;
    int index = 0;
    while (std::getline(row_stream, value, delimiter)) {
      function(value, index);
      index++;
    }
    get_line_succeeded = true;
  }
  return get_line_succeeded;
}

bool csv_log_reader::read_header() {
  return for_each_value_of_next_line(m_stream, m_delimiter,
                                     [&](std::string &value, int index) {
                                       remove_quotes(value);
                                       save_column_index(value, index);
                                     });
}

std::optional<traffic_log> csv_log_reader::get_line() {

  // if first line, will retreive the data format
  if (m_first_row) {
    if (!read_header()) {
      // impossible to read the first line with data format
      return std::nullopt;
    }
    m_first_row = false;
  };

  traffic_log result;
  bool get_line_succeeded = for_each_value_of_next_line(
      m_stream, m_delimiter, [&](std::string &value, int index) {
        remove_quotes(value);
        fill_log(result, value, index);
      });

  if (!get_line_succeeded) {
    return std::nullopt;
  }

  return std::move(result);
}

} // namespace monitoring
