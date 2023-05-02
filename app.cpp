// async_consume.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT consumer/subscriber using the C++
// asynchronous client interface, employing the  to receive messages
// and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the synchronous queuing API
//

#include "mqtt/async_client.h"
#include "spdlog/spdlog.h"
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include "common.hpp"

using namespace std;

// declare the mqtt thread
extern void *mqtt_thread(void *arg);

// the function setup the dormitory region
// setup the dormitory region,
void dormitory_setup(void) {

  auto dormitory = dormitoryIOT::get_instance();

  // create systems belong to the region
  lighting lighting_led("led", "led_id", true);
  security security_smoke("smoke", "smoke_id", true);
  // create the devices belong to this systems
  led led1("led1", "led1_id", "off", 0);
  dth11 dth11_1("dth11_1", "dth11_1_id", 0, 0);
  mq2 mq2_1("mq2_1", "mq2_1_id", 0, "o");

  // add the devices to the systems
  lighting_led.add_device(&led1);
  security_smoke.add_device(&dth11_1);
  security_smoke.add_device(&mq2_1);

  // add the systems to the region
  dormitory->add_system(&lighting_led);
  dormitory->add_system(&security_smoke);
  dormitory->set_security_system(&security_smoke);

  // set the lighting system groups
  auto devices = list<device *>{&led1};
  lighting_led.add_group("dormitory", devices);
}

// the thread create a reagion and handle the message from the reagion
int main(int argc, char *argv[]) {

  // setting the log
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] %v");

  // create the region
  dormitory_setup();

  // get the region
  auto dormitory = dormitoryIOT::get_instance();

  // create all threads for the region
  dormitory->region_thread(&dormitory);

  // the main thread is not exit, because it create three thread belong to
  // dormitory

  // create the mqtt thread
  pthread_t mqtt_thread_handler;
  pthread_create(&mqtt_thread_handler, nullptr, mqtt_thread, &dormitory);

  while (true) {
    spdlog::debug("main thread is running");
    usleep(1000000);
  }
}
