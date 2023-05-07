#ifndef __DORMITORY_HPP__
#define __DORMITORY_HPP__

#include "mqtt/async_client.h"
#include "pthread.h"
#include "spdlog/spdlog.h"
#include <list>

#include "message.hpp"
#include "threadpool.hpp"

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

// declare functions
void *control_panel_thread(void *arg);
void *security_thread(void *arg);

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
  virtual void control_panel(void *arg){};     // control the system
  void message_handler(void *arg) { return; }; // message callback function
};

class region {
public:
  std::list<systemIOT *> systems;
  systemIOT *security_system;
  bool has_security_update;

public:
  region(std::string region_name, std::string region_id)
      : region_name(region_name), region_id(region_id),
        has_security_update(false) {}
  std::string get_region_name() const { return region_name; }
  std::string get_region_id() const { return region_id; }
  std::list<systemIOT *> get_systems() { return systems; }
  systemIOT *get_system();
  void add_system(systemIOT *s) { systems.push_back(s); }
  void delete_system(systemIOT *s) { systems.remove(s); }
  void set_security_system(systemIOT *s) { security_system = s; }
  systemIOT *get_security_system() { return security_system; }
  systemIOT *update_security_system(systemIOT *s) {
    return security_system = s;
  }

  bool get_security_update() const { return has_security_update; }
  virtual void region_thread(void *arg) = 0;

private:
  std::string region_name;
  std::string region_id;
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
  void control_panel(void *arg);
  static void message_handler(void *arg);
};

class security : public systemIOT {
  std::string security_status;

public:
  security(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system),
        security_status("no") {}
  std::string get_security_status() const { return security_status; }
  void control_panel(void *arg); // control the security system
  static void message_handler(void *arg);
};

class dormitoryIOT : public region {
public:
  ThreadPool pool;

public:
  static dormitoryIOT *GetInstance();
  static void DeleteInstance();
  void print();

  void region_thread(void *arg);
  systemIOT *get_system(std::string system_id);

private:
  dormitoryIOT(std::string region_name, std::string region_id);
  ~dormitoryIOT();

  dormitoryIOT(const dormitoryIOT &);
  const dormitoryIOT &operator=(const dormitoryIOT &);

private:
  static dormitoryIOT *instance;
};

#endif