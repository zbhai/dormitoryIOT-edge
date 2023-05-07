#include "message.hpp"
#include "dormitory.hpp"

// create the global message distributor
void *distrbutor;

message EMPTY_MESSAGE("", "");

/* implement the message method */
int message_queue::push(std::string &data, std::string &topic) {
  if (length < max_size) {
    pthread_mutex_lock(&mutex);
    messages.push_back(message(data, topic));
    pthread_mutex_unlock(&mutex);
    length++;
    spdlog::debug("push message to queue: {} {} {}", data, topic, length);
    return 0;
  }
  return -1;
}
message message_queue::pop() {
  if (messages.empty())
    return message("", "");
  pthread_mutex_lock(&mutex);
  message m = messages.front();
  messages.pop_front();
  pthread_mutex_unlock(&mutex);
  length--;
  spdlog::debug("pop message from queue: {} {} {}", m.get_data(), m.get_topic(),
                length);
  return m;
}

// message module initialization function
template <int key, typename R, typename... Args> R Dispatch(Args... args) {
  auto m_type =
      std::make_tuple(MESSAGE_LIGHTING, MESSAGE_SECURITY); ///< 消息类型 tuple
  auto i_type = std::make_tuple(lighting::message_handler,
                                security::message_handler); ///< 处理函数 tuple
  auto m_dis = Cosmos::Zip(m_type, i_type);                 ///< 合并二者

  return Cosmos::Apply(std::get<key>(m_dis).second, args...);
}
