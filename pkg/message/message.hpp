#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include "mqtt/async_client.h"
#include "pthread.h"
#include "spdlog/spdlog.h"
#include <list>





enum {
  MESSAGE_SECURITY,
  MESSAGE_LIGHTING,
};

class message {
  std::string data;
  std::string topic;
  int m_type;

public:
  message(const std::string &data, const std::string &topic) noexcept 
      : data(data), topic(topic) {}
  message() noexcept {}
  
  message(const message &m) noexcept : data(m.data), topic(m.topic) {}
  message &operator=(const message &m) noexcept {
    data = m.data;
    topic = m.topic;
    return *this;
  }
  
  message(message &&m) noexcept : data(std::move(m.data)), topic(std::move(m.topic))  {}
  message &operator=(message &&m) noexcept {
    data = std::move(m.data);
    topic = std::move(m.topic);
    return *this;
  }
  std::string get_data() const { return data; }
  std::string get_topic() const { return topic; }
};



#endif // __MESSAGE_HPP__