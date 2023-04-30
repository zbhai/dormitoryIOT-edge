#ifndef __COMMON_H__
#define __COMMON_H__

#include "mqtt/async_client.h"
#include "pthread.h"
#include "spdlog/spdlog.h"
#include <list>

const std::string SERVER_ADDRESS{"localhost:1883"};
const std::string CLIENT_ID{"dormitoryIOT"};
const std::string TOPIC{"dormitoryIOT/test"};
const std::string REC_DEVICE_TOPIC{"$ke/events/device/+/data/update"};
const std::string SEN_DEVICE_TOPIC{"$hw/events/device/+/twin/update/delta"};
const std::string SEN_CLOUD_TOPIC{"SYS/dis/upload_records"};

// declare functions
void *control_panel_thread(void *arg);
void *security_thread(void *arg);
void *handle_message_thread(void *arg);

class message {
  std::string data;
  std::string topic;

public:
  message(const std::string &data, const std::string &topic)
      : data(data), topic(topic) {}
  std::string get_data() const { return data; }
  std::string get_topic() const { return topic; }
};

class message_queue {
  std::list<message> messages;
  int length;
  int max_size;

  pthread_mutex_t mutex;

public:
  message_queue(int max_size) : messages(), length(0), max_size(max_size) {
    pthread_mutex_init(&mutex, NULL);
  }
  ~message_queue() { pthread_mutex_destroy(&mutex); }
  int push(std::string &data, std::string &topic) {
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
  message pop() {
    if (messages.empty())
      return message("", "");
    pthread_mutex_lock(&mutex);
    message m = messages.front();
    messages.pop_front();
    pthread_mutex_unlock(&mutex);
    length--;
    spdlog::debug("pop message from queue: {} {} {}", m.get_data(),
                  m.get_topic(), length);
    return m;
  }
  bool empty() { return messages.empty(); }
  int size() { return messages.size(); }
};

message EMPTY_MESSAGE("", "");

// define the device message model
//---------------LED----------------
// led status: off/on               ; the led status is read and write
// led pin number: uint8_t          ; the led pin number is read
//---------------LED----------------
// the led status will be write to the device, if the nother things is occur.

// --------------DTH11--------------
// termperature: uint8_t            ; the temperature is read
// humidity: uint8_t                ; the humidity is read
// --------------DTH11--------------
// only show the temperature and humidity in screen

// --------------MQ2---------------
// smoke value: uint8_t             ; the smoke value is read
// alarm status: off/on             ; the alarm status is read and write
// --------------MQ2---------------
// the alarm status will be write to the device, if the nother things is occur.

class systemIOT;

class region {
  std::string region_name;
  std::string region_id;

public:
  std::list<systemIOT *> systems;
  systemIOT *security_system;
  bool has_security_update;

  region(std::string region_name, std::string region_id)
      : region_name(region_name), region_id(region_id),
        has_security_update(false) {}
  std::string get_region_name() const { return region_name; }
  std::string get_region_id() const { return region_id; }
  std::list<systemIOT *> get_systems() { return systems; }
  void add_system(systemIOT *s) { systems.push_back(s); }
  void delete_system(systemIOT *s) { systems.remove(s); }
  void set_security_system(systemIOT *s) { security_system = s; }
  systemIOT *get_security_system() { return security_system; }
  systemIOT *update_security_system(systemIOT *s) {
    return security_system = s;
  }
  bool get_security_update() const { return has_security_update; }

  virtual void region_thread(void *arg) = 0;
};

class dormitoryIOT : public region {

private:
  static dormitoryIOT *instance;

  dormitoryIOT(std::string region_name, std::string region_id)
      : region(region_name, region_id) {}

public:
  dormitoryIOT *Getinstance(std::string region_name, std::string region_id,
                            int pub_size, int sub_size) {
    if (instance == nullptr) {
      static dormitoryIOT diot(region_name, region_id);
      instance = &diot;
      return instance;
    } else {
      return instance;
    }
  }
  void region_thread(void *arg) {
    // first, create a thread to handle the security system
    dormitoryIOT *dormitory = (dormitoryIOT *)arg;

    pthread_t security_thread_handler;
    pthread_create(&security_thread_handler, NULL, security_thread, dormitory);

    // fourth, create a thread to handle the control panel
    pthread_t control_panel_thread_handler;
    pthread_create(&control_panel_thread_handler, NULL, control_panel_thread,
                   dormitory);
  }
};

class device {
  std::string device_name;
  std::string device_id;

public:
  device(std::string device_name, std::string device_id)
      : device_name(device_name), device_id(device_id) {}
  std::string get_name() const { return device_name; }
  std::string get_id() const { return device_id; }
  virtual bool synced() = 0;
};

class led : public device {
  std::string status;
  std::string desired_status;
  uint8_t pin_number;

public:
  led(std::string device_name, std::string device_id, std::string status,
      uint8_t pin_number)
      : device(device_name, device_id), status(status), pin_number(pin_number) {
  }
  std::string get_status() const { return status; }
  message set_desired_status(
      std::string desired); // create a task and be executed later
  bool synced() { return status == desired_status; }
};
inline message led::set_desired_status(std::string desired) {
  // create a message and push to the message queue
  std::string topic(SEN_DEVICE_TOPIC);
  topic.replace(topic.find("+"), 1, "device_id");
  return message(desired, topic);
}

class dth11 : public device {
  uint8_t temperature;
  uint8_t humidity;

public:
  dth11(std::string device_name, std::string device_id, uint8_t temperature,
        uint8_t humidity)
      : device(device_name, device_id), temperature(temperature),
        humidity(humidity) {}
  uint8_t get_temporary() const { return temperature; }
  uint8_t get_humidity() const { return humidity; }
  bool synced() { return true; }
};

class mq2 : public device {
  uint8_t smoke_value;
  std::string alarm_status;
  std::string desired_alarm_status;

public:
  mq2(std::string device_name, std::string device_id, uint8_t smoke_value,
      std::string alarm_status)
      : device(device_name, device_id), smoke_value(smoke_value),
        alarm_status(alarm_status) {}
  uint8_t get_smoke_value() const { return smoke_value; }
  std::string get_alarm_status() const { return alarm_status; }
  void set_desired_alarm_status(std::string desired) const {
    return;
  } // create a task and be executed later
  bool synced() { return alarm_status == desired_alarm_status; }
};

class systemIOT {
  bool is_basic_system;
  std::string system_name;
  std::string system_id;
  std::list<device *> devices;
  std::list<systemIOT *> subsystems;
  std::list<systemIOT *> related_systems;

public:
  systemIOT(std::string system_name, std::string system_id,
            bool is_basic_system)
      : system_name(system_name), system_id(system_id),
        is_basic_system(is_basic_system) {}
  std::string get_name() const { return system_name; }
  std::string get_id() const { return system_id; }
  bool is_basic() const { return is_basic_system; }
  std::list<device *> get_devices() const { return devices; }
  std::list<systemIOT *> get_subsystems() const { return subsystems; }
  std::list<systemIOT *> get_related_systems() const { return related_systems; }
  void add_device(device *d) { devices.push_back(d); }
  void add_subsystem(systemIOT *s) { subsystems.push_back(s); }
  void add_related_system(systemIOT *s) { related_systems.push_back(s); }
  virtual void control_panel(void *arg){}; // control the system
};

class lighting : public systemIOT {
  std::map<std::string, std::list<device *>> lighting_groups;

public:
  lighting(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system) {}
  std::map<std::string, std::list<device *>> get_groups() const {
    return lighting_groups;
  }
  void add_group(std::string group_name, std::list<device *> devices) {
    lighting_groups[group_name] = devices;
  }
  inline void control_panel(void *arg) {
    // get the dormitory
    auto dormitory = (dormitoryIOT *)arg;

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
    // turn on/off the light based on the motion sensor now the motion sensor
    // is empty
  }
};

class security : public systemIOT {
  std::string security_status;

public:
  security(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system),
        security_status("no") {}
  std::string get_security_status() const { return security_status; }
  void control_panel(void *arg); // control the security system
};
inline void security::control_panel(void *arg) {
  // control the security system
  // now, the control panel is empty
}

void *security_thread(void *arg) { spdlog::debug("security_thread start"); }

void *control_panel_thread(void *arg) {
  spdlog::debug("control_panel_thread start");
}

#endif