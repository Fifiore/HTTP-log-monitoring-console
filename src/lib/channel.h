#pragma once
#include <optional>
#include <mutex>
#include <deque>
#include <condition_variable>

namespace monitoring {
  template <class T>
  class channel {
  public:
    // channel is closed
    ~channel() {
      close();
    }

    // Value in channel available for receiver
    void send(T value) {
      if (m_closed)
        return;

      std::scoped_lock<std::mutex> hold(m_mutex);
      m_fifo.push_back(std::move(value));
      m_condition_variable.notify_all();
    }

    // Receive a value from a sender.
    // `nullopt` means the channel is now closed.
    std::optional<T> receive() {
      std::scoped_lock<std::mutex> hold(m_mutex);

      m_condition_variable.wait(m_mutex, [&] {
        return !m_fifo.empty() || m_closed; 
        });

      if (m_closed && m_fifo.empty())
        return std::nullopt;

      T value = m_fifo.front();
      m_fifo.pop_front();
      return std::move(value);
    }

    // Close the channel.
    void close() {
      m_closed = true;
      m_condition_variable.notify_all();
    }

  private:
    std::mutex m_mutex;
    std::condition_variable_any m_condition_variable;
    std::deque<T> m_fifo;
    std::atomic<bool> m_closed{ false };
  };
}

