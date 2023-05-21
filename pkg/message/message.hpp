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
  message(const std::string &data, const std::string &topic)
      : data(data), topic(topic) {}
  message() {}
  
  message(const message &m) : data(m.data), topic(m.topic) {}
  message &operator=(const message &m) {
    data = m.data;
    topic = m.topic;
    return *this;
  }
  
  message(message &&m) : data(std::move(m.data)), topic(std::move(m.topic)) {}
  message &operator=(message &&m) {
    data = std::move(m.data);
    topic = std::move(m.topic);
    return *this;
  }
  std::string get_data() const { return data; }
  std::string get_topic() const { return topic; }
};

class message_queue {
  std::list<message> messages;
  int length;
  int max_size;

  pthread_mutex_t mutex;

public:
  message_queue(int max_size) :  length(0), max_size(max_size) {
    pthread_mutex_init(&mutex, NULL);
  }
  ~message_queue() { pthread_mutex_destroy(&mutex); }
  int push(std::string &data, std::string &topic);
  message pop();
  bool empty() { return (length != 0); }
  int size() { return length; }
};

#endif // __MESSAGE_HPP__