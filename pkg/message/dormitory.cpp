#include "dormitory.hpp"
#include "message.hpp"

// creating the global message queue
message_queue lighting_queue(50);
message_queue security_queue(10);

/* implement systemIOT method */
device *systemIOT::get_device(std::string device_id) {
  device *d = NULL;

  pthread_rwlock_rdlock(&device_lock);
  for (auto i : devices) {
    if (i->get_id() == device_id) {
      d = i;
      break;
    }
  }
  pthread_rwlock_unlock(&device_lock);

  return d;
}
void systemIOT::add_device(device *d) {
  pthread_rwlock_wrlock(&device_lock);
  devices.push_back(d);
  pthread_rwlock_unlock(&device_lock);
}

systemIOT *systemIOT::get_subsystem(std::string subsystem_id) {
  systemIOT *s = NULL;

  pthread_rwlock_rdlock(&subsystem_lock);
  for (auto i : subsystems) {
    if (i->get_id() == subsystem_id) {
      s = i;
      break;
    }
  }
  pthread_rwlock_unlock(&subsystem_lock);

  return s;
}
void systemIOT::add_subsystem(systemIOT *s) {
  pthread_rwlock_wrlock(&subsystem_lock);
  subsystems.push_back(s);
  pthread_rwlock_unlock(&subsystem_lock);
}

/* implement the region method */
systemIOT *systemIOT::get_related_system(std::string related_system_id) {
  systemIOT *s = NULL;

  pthread_rwlock_rdlock(&related_system_lock);
  for (auto i : related_systems) {
    if (i->get_id() == related_system_id) {
      s = i;
      break;
    }
  }
  pthread_rwlock_unlock(&related_system_lock);

  return s;
}
void systemIOT::add_related_system(systemIOT *s) {
  pthread_rwlock_wrlock(&related_system_lock);
  related_systems.push_back(s);
  pthread_rwlock_unlock(&related_system_lock);
}

systemIOT *region::get_system(std::string system_id) {
  pthread_rwlock_rdlock(&system_lock);
  spdlog::debug("region get system location00");
  for (auto it = systems.begin(); it != systems.end(); it++) {
    if ((*it)->get_id() == system_id,
        spdlog::debug("region get system location0.5"),
        (*it)->get_id() == system_id) {
      spdlog::debug("region get system location01");
      pthread_rwlock_unlock(&system_lock);
      return *it;
    }
  }
  spdlog::debug("region get system location02");
  pthread_rwlock_unlock(&system_lock);
  return NULL;
}
void region::add_system(systemIOT *s) {
  pthread_rwlock_wrlock(&system_lock);
  systems.push_back(s);
  pthread_rwlock_unlock(&system_lock);
}
void region::delete_system(systemIOT *s) {
  pthread_rwlock_wrlock(&system_lock);
  systems.remove(s);
  pthread_rwlock_unlock(&system_lock);
}

systemIOT *region::update_security_system(systemIOT *s) {
  pthread_rwlock_wrlock(&security_system_lock);
  security_system = s;
  pthread_rwlock_unlock(&security_system_lock);
  return security_system;
}
systemIOT *region::get_security_system() {
  pthread_rwlock_rdlock(&security_system_lock);
  systemIOT *s = security_system;
  pthread_rwlock_unlock(&security_system_lock);
  return s;
}

/* implement the led method */
std::string led::get_status() {
  pthread_rwlock_rdlock(&ledlock);
  std::string temp = status;
  pthread_rwlock_unlock(&ledlock);
  return temp;
}
extern const std::string SEN_DEVICE_TOPIC{
    "hw/events/device/+/twin/update/delta"};
message led::set_desired_status(std::string desired) {
  pthread_rwlock_wrlock(&ledlock);
  desired_status = desired;
  pthread_rwlock_unlock(&ledlock);
  return message(desired, SEN_DEVICE_TOPIC);
}
bool led::synced() {
  pthread_rwlock_rdlock(&ledlock);
  bool flag = status == desired_status;
  pthread_rwlock_unlock(&ledlock);
  return flag;
}
void led::set_status(std::string status) {
  pthread_rwlock_wrlock(&ledlock);
  this->status = status;
  pthread_rwlock_unlock(&ledlock);
}
void led::get_desired_status(std::string desired_status) {
  pthread_rwlock_rdlock(&ledlock);
  desired_status = this->desired_status;
  pthread_rwlock_unlock(&ledlock);
}

/* implement the dth11 device method */
uint8_t dth11::get_temperature() {
  pthread_rwlock_rdlock(&dth11lock);
  uint8_t temp = this->temperature;
  pthread_rwlock_unlock(&dth11lock);
  return temp;
}
uint8_t dth11::get_humidity() {
  pthread_rwlock_rdlock(&dth11lock);
  uint8_t hum = this->humidity;
  pthread_rwlock_unlock(&dth11lock);
  return hum;
}
bool dth11::synced() { return true; }
void dth11::set_temperature(uint8_t temperature) {
  pthread_rwlock_wrlock(&dth11lock);
  this->temperature = temperature;
  pthread_rwlock_unlock(&dth11lock);
}
void dth11::set_humidity(uint8_t humidity) {
  pthread_rwlock_wrlock(&dth11lock);
  this->humidity = humidity;
  pthread_rwlock_unlock(&dth11lock);
}

/* implement the mq2 device method */
uint8_t mq2::get_smoke_value() {
  pthread_rwlock_rdlock(&mq2lock);
  uint8_t smoke_value = this->smoke_value;
  pthread_rwlock_unlock(&mq2lock);
  return smoke_value;
}
std::string mq2::get_alarm_status() {
  pthread_rwlock_rdlock(&mq2lock);
  std::string alarm_status = this->alarm_status;
  pthread_rwlock_unlock(&mq2lock);
  return alarm_status;
}
void mq2::set_desired_alarm_status(std::string desired) {
  pthread_rwlock_wrlock(&mq2lock);
  this->desired_alarm_status = desired;
  pthread_rwlock_unlock(&mq2lock);
}
bool mq2::synced() {
  pthread_rwlock_rdlock(&mq2lock);
  bool synced = (this->alarm_status == this->desired_alarm_status);
  pthread_rwlock_unlock(&mq2lock);
  return synced;
}
void mq2::set_smoke_value(uint8_t smoke_value) {
  pthread_rwlock_wrlock(&mq2lock);
  this->smoke_value = smoke_value;
  pthread_rwlock_unlock(&mq2lock);
}
void mq2::set_alarm_status(std::string alarm_status) {
  pthread_rwlock_wrlock(&mq2lock);
  this->alarm_status = alarm_status;
  pthread_rwlock_unlock(&mq2lock);
}
void mq2::get_desired_alarm_status(std::string desired_alarm_status) {
  pthread_rwlock_rdlock(&mq2lock);
  desired_alarm_status = this->desired_alarm_status;
  pthread_rwlock_unlock(&mq2lock);
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
std::string security::get_security_status() {
  std::string status;
  pthread_rwlock_rdlock(&security_status_lock);
  status = security_status;
  pthread_rwlock_unlock(&security_status_lock);
  return status;
}
void security::set_security_status(std::string security_status) {
  pthread_rwlock_wrlock(&security_status_lock);
  this->security_status = security_status;
  pthread_rwlock_unlock(&security_status_lock);
}
void security::control_panel(void *arg) {
  auto dormitory = dormitoryIOT::GetInstance();
  static int task_flag = 0;

  if (!task_flag) {
    // create a task to handle the message
    Task Task(security_message_handler, nullptr);
    dormitory->pool.addTask(Task);
    task_flag = 1;
  }
}

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
std::list<device *> lighting::get_group(std::string group_name) {
  std::list<device *> devices;
  pthread_rwlock_rdlock(&lighting_groups_lock);
  if (lighting_groups.find(group_name) != lighting_groups.end()) {
    devices = lighting_groups[group_name];
  }
  pthread_rwlock_unlock(&lighting_groups_lock);
  return devices;
}
void lighting::add_group(std::string group_name, std::list<device *> devices) {
  pthread_rwlock_wrlock(&lighting_groups_lock);
  lighting_groups[group_name] = devices;
  pthread_rwlock_unlock(&lighting_groups_lock);
}
void lighting::control_panel(void *arg) {
  // get the dormitory
  auto dormitory = dormitoryIOT::GetInstance();
  static int task_flag = 0;

  spdlog::debug("lighting control panel location01");
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

dormitoryIOT::dormitoryIOT(std::string region_name, std::string region_id)
    : pool(1, 4), region(region_name, region_id) {}
dormitoryIOT::~dormitoryIOT() {}

/* implement region task */
void *security_thread(void *arg) {
  spdlog::debug("security_thread start");
  return nullptr;
}

void *control_panel_thread(void *arg) {
  spdlog::debug("control_panel_thread start");
  auto dormitory = dormitoryIOT::GetInstance();

  while (true) {
    spdlog::debug("control_panel_thread location01");
    // get the lighting system
    std::string s("lighting");
    systemIOT *lighting = dormitory->get_system(s);
    spdlog::debug("control_panel_thread location02");
    if (lighting) {
      lighting->control_panel(nullptr);
    }
    usleep(1000000);
  }

  return nullptr;
}