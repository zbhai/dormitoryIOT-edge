#ifndef __DORMITORY_HPP__
#define __DORMITORY_HPP__

#include "mqtt/async_client.h"
#include "pthread.h"
#include "spdlog/spdlog.h"
#include <list>

#include "message.hpp"
#include "mqtt.hpp"
#include "threadpool.hpp"



#define MAX_GENERAL_QUEUE_SIZE (100)
#define MAX_LIGHTING_QUEUE_SIZE MAX_GENERAL_QUEUE_SIZE
#define MAX_SECURITY_QUEUE_SIZE (20)

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
  std::string get_name();
  std::string get_id();
  virtual bool synced() = 0;
};

class systemIOT {
private:
  std::string system_name;
  std::string system_id;

   bool is_basic_system;

  std::list<device *> devices;
  std::list<systemIOT *> subsystems;
  std::list<systemIOT *> related_systems;

  pthread_rwlock_t device_lock;
  pthread_rwlock_t subsystem_lock;
  pthread_rwlock_t related_system_lock;

public:
  systemIOT(std::string system_name, std::string system_id,
            bool is_basic_system)
      : system_name(system_name), system_id(system_id),
        is_basic_system(is_basic_system) {
    pthread_rwlock_init(&device_lock, NULL);
    pthread_rwlock_init(&subsystem_lock, NULL);
    pthread_rwlock_init(&related_system_lock, NULL);
  }
  std::string get_name();
  std::string get_id();
  bool is_basic();

  device *get_device(std::string);
  void add_device(device *d);

  systemIOT *get_subsystem(std::string);
  void add_subsystem(systemIOT *s);

  systemIOT *get_related_system(std::string);
  void add_related_system(systemIOT *s);

  virtual void control_panel(void *arg) = 0; // control the system
};


class region {
private:
  std::list<systemIOT *> systems;
  systemIOT *security_system;

  pthread_rwlock_t system_lock;
  pthread_rwlock_t security_system_lock;

public:
  region(std::string region_name, std::string region_id)
      : region_name(region_name), region_id(region_id) {
    pthread_rwlock_init(&system_lock, NULL);
    pthread_rwlock_init(&security_system_lock, NULL);
  }

  std::string get_name();
  std::string get_id();

  systemIOT *get_system(std::string);
  void add_system(systemIOT *s);
  void delete_system(systemIOT *s);

  systemIOT *get_security_system();
  systemIOT *update_security_system(systemIOT *s);

  virtual void region_thread(void *arg) = 0;

private:
  std::string region_name;
  std::string region_id;
};


class led : public device {
  std::string status;
  std::string desired_status;
  uint8_t pin_number;

  pthread_rwlock_t ledlock;

public:
  led(std::string device_name, std::string device_id, std::string status,
      uint8_t pin_number)
      : device(device_name, device_id), status(status), pin_number(pin_number) {
    pthread_rwlock_init(&ledlock, NULL);
  }
  std::string get_status();
  message set_desired_status(
      std::string desired); // create a task and be executed later
  bool synced();

private:
  void set_status(std::string status);
  void get_desired_status(std::string desired_status);
};

class dth11 : public device {
  uint8_t temperature;
  uint8_t humidity;

  pthread_rwlock_t dth11lock;

public:
  dth11(std::string device_name, std::string device_id, uint8_t temperature,
        uint8_t humidity)
      : device(device_name, device_id), temperature(temperature),
        humidity(humidity) {
    pthread_rwlock_init(&dth11lock, NULL);
  }
  uint8_t get_temperature();
  uint8_t get_humidity();
  bool synced();

private:
  void set_temperature(uint8_t temperature);
  void set_humidity(uint8_t humidity);
};

class mq2 : public device {
  uint8_t smoke_value;
  std::string alarm_status;
  std::string desired_alarm_status;

  pthread_rwlock_t mq2lock;

public:
  mq2(std::string device_name, std::string device_id, uint8_t smoke_value,
      std::string alarm_status)
      : device(device_name, device_id), smoke_value(smoke_value),
        alarm_status(alarm_status) {
    pthread_rwlock_init(&mq2lock, NULL);
  }
  uint8_t get_smoke_value();
  std::string get_alarm_status();
  void set_desired_alarm_status(std::string desired);
  bool synced();

private:
  void set_smoke_value(uint8_t smoke_value);
  void set_alarm_status(std::string alarm_status);
  void get_desired_alarm_status(std::string desired_alarm_status);
};

class lighting : public systemIOT {
  std::map<std::string, std::list<device *>> lighting_groups;

  pthread_rwlock_t lighting_groups_lock;

public:
  lighting(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system) {
    pthread_rwlock_init(&lighting_groups_lock, NULL);
  }

  std::list<device *> get_group(std::string group_name);
  void add_group(std::string group_name, std::list<device *> devices);
  void control_panel(void *arg);
};

class security : public systemIOT {
  std::string security_status;

  pthread_rwlock_t security_status_lock;

public:
  security(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system),
        security_status("no") {
    pthread_rwlock_init(&security_status_lock, NULL);
  }
  std::string get_security_status();

  void control_panel(void *arg); // control the security system

private:
  void set_security_status(std::string security_status);
};

class general : public systemIOT{

  pthread_rwlock_t general_lock;

  public:
  general(std::string system_name, std::string system_id, bool is_basic_system)
      : systemIOT(system_name, system_id, is_basic_system) {
    pthread_rwlock_init(&general_lock, NULL);
  }

  void control_panel(void *arg);
};

class dormitoryIOT : public region {
public:
  ThreadPool pool;

public:
  static dormitoryIOT *GetInstance();
  static void DeleteInstance();
  void print();

  void region_thread(void *arg);

private:
  dormitoryIOT(std::string region_name, std::string region_id);
  ~dormitoryIOT();

  dormitoryIOT(const dormitoryIOT &);
  const dormitoryIOT &operator=(const dormitoryIOT &);

private:
  static dormitoryIOT *instance;
};

#endif