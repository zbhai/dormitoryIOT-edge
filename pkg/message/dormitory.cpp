#include "dormitory.hpp"
#include "message.hpp"

// creating the global message queue
message_queue lighting_queue(50);
message_queue security_queue(10);

/* implement the dormitoryIOT method */
dormitoryIOT *dormitoryIOT::instance =
    new (std::nothrow) dormitoryIOT("dormitory", "dormitory");
dormitoryIOT *dormitoryIOT::GetInstance() { return instance; }
void dormitoryIOT::DeleteInstance() {
  if (instance) {
    delete instance;
    instance = nullptr;
  }
}
void dormitoryIOT::print() {
  std::cout << "the instance address is:" << this << std::endl;
}
void dormitoryIOT::region_thread(void *arg) {
  // first, create a thread to handle the security system
  dormitoryIOT *dormitory = (dormitoryIOT *)arg;

  pthread_t security_thread_handler;
  pthread_create(&security_thread_handler, NULL, security_thread, dormitory);

  // fourth, create a thread to handle the control panel
  pthread_t control_panel_thread_handler;
  pthread_create(&control_panel_thread_handler, NULL, control_panel_thread,
                 dormitory);
}
systemIOT *dormitoryIOT::get_system(std::string system_id) {
  if (system_id == "security") {
    return security_system;
  }
  for (auto system : systems) {
    if (system->get_id() == system_id) {
      return system;
    }
  }
  return nullptr;
}
dormitoryIOT::dormitoryIOT(std::string region_name, std::string region_id)
    : pool(1, 4), region(region_name, region_id) {}
dormitoryIOT::~dormitoryIOT() {}

/* implement the lighting method */
void lighting_message_handler(void *arg) {
  message_queue &m_queue = *(message_queue *)arg;

  while (true) {
    if (!m_queue.empty()) {
      message m = m_queue.pop();
      if (m.get_topic() == "lighting") {
        spdlog::debug("lighting message: {}", m.get_data());
      }
    }
    usleep(1000);
  }
}

void lighting::control_panel(void *arg) {
  // get the dormitory
  auto dormitory = dormitoryIOT::GetInstance();
  static int task_flag = 0;

  if (!task_flag) {
    // create a task to handle the message
    Task Task(lighting_message_handler, nullptr);
    dormitory->pool.addTask(Task);
    task_flag = 1;
  }

  // turn on/off the light based on the time
  auto origin_time = std::time(nullptr);
  auto local_time = std::localtime(&origin_time);
  auto hour = local_time->tm_hour;  // int
  auto minute = local_time->tm_min; // int

  // the time control panel is based on groups
  // assert the lighting_groups is not empty
  if (lighting_groups.empty()) {
    return;
  }
  auto iter = lighting_groups.find("dormitory");
  auto devices = iter->second;
  for (auto device : devices) {
    if (hour >= 23 || hour <= 6) {
      // turn on the light
      led *lighting = dynamic_cast<led *>(device);
      spdlog::debug("turn on the light");
      auto m = lighting->set_desired_status("on");
    } else {
      // turn off the light
      led *lighting = dynamic_cast<led *>(device);
      spdlog::debug("turn off the light");
      auto m = lighting->set_desired_status("off");
    }
  }
}

/* implement the security method */
void security_message_handler(void *arg) {
  message_queue &m_queue = *(message_queue *)arg;

  while (true) {
    if (!m_queue.empty()) {
      message m = m_queue.pop();
      if (m.get_topic() == "security") {
        spdlog::debug("security message: {}", m.get_data());
      }
    }
    usleep(1000);
  }
}
void security::control_panel(void *arg) {
  auto dormitory = dormitoryIOT::GetInstance();
  static int task_flag = 0;

  if (!task_flag) {
    // create a task to handle the message
    Task Task(lighting_message_handler, nullptr);
    dormitory->pool.addTask(Task);
    task_flag = 1;
  }
}

/* implement the led deivce method */
message led::set_desired_status(std::string desired) {
  // create a message and push to the message queue
  std::string topic("device/+/led");
  topic.replace(topic.find("+"), 1, "device_id");
  return message(desired, topic);
}

/* implement threads */
void *security_thread(void *arg) {
  spdlog::debug("security_thread start");
  return nullptr;
}

void *control_panel_thread(void *arg) {
  spdlog::debug("control_panel_thread start");
  // auto dormitory = dormitoryIOT::GetInstance();

  // while (true) {
  //   spdlog::debug("control_panel_thread location01");
  //   for (auto system : dormitory->systems) {
  //     system->control_panel(nullptr);
  //   }
  //   usleep(1000);
  // }

  return nullptr;
}