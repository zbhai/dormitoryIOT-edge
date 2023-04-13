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

const int QOS = 1;

/////////////////////////////////////////////////////////////////////////////

void *mqtt_thread(void *arg) {

  spdlog::debug("start the mqtt thread");
  // get dormitory
  auto dormitory = (dormitoryIOT *)arg;

  spdlog::debug("start to create the mqtt client");

  // create the mqtt client
  mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

  auto connOpts =
      mqtt::connect_options_builder().clean_session(false).finalize();

  try {
    // Start consumer before connecting to make sure to not miss messages

    cli.start_consuming();

    // Connect to the server

    spdlog::debug("connecting to the mqtt server");
    auto tok = cli.connect(connOpts);

    // Getting the connect response will block waiting for the
    // connection to complete.
    auto rsp = tok->get_connect_response();

    // If there is no session present, then we need to subscribe, but if
    // there is a session, then the server remembers us and our
    // subscriptions.
    if (!rsp.is_session_present())
      cli.subscribe(TOPIC, QOS)->wait();

    spdlog::debug("connected to the mqtt server");

    // Consume messages
    // This just exits if the client is disconnected.
    // (See some other examples for auto or manual reconnect)

    spdlog::debug("start to consume the message");

    while (true) {
      auto msg = cli.consume_message();
      if (!msg)
        break;
      // add the message to the sub_topics queue
      std::string data = static_cast<std::string>(msg->get_payload());
      std::string topic = static_cast<std::string>(msg->get_topic());
      spdlog::debug("message topic: {}, message data: {}", topic, data);
      dormitory->push_message(data, topic);
      // debug the message information
    }

    // If we're here, the client was almost certainly disconnected.
    // But we check, just to make sure.

    if (cli.is_connected()) {
      cout << "\nShutting down and disconnecting from the MQTT server..."
           << flush;
      cli.unsubscribe(TOPIC)->wait();
      cli.stop_consuming();
      cli.disconnect()->wait();
      cout << "OK" << endl;
    } else {
      cout << "\nClient was disconnected" << endl;
    }
  } catch (const mqtt::exception &exc) {
    cerr << "\n  " << exc << endl;
    exit(-1);
  }

  exit(0);
}

// the thread create a reagion and handle the message from the reagion
int main(int argc, char *argv[]) {

  // setting the log
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] %v");
  // log the information
  spdlog::debug("start the mqtt client");

  // create the region
  dormitoryIOT dormitory("dormitory614", "dormitory614_id", 100, 100);
  // create systems belong to the region
  lighting lighting_led("led", "led_id", true);
  security security_smoke("smoke", "smoke_id", true);
  // create the devices belong to this systems
  led led1("led1", "led1_id", "off", 0);
  dth11 dth11_1("dth11_1", "dth11_1_id", 0, 0);
  mq2 mq2_1("mq2_1", "mq2_1_id", 0, "o");

  spdlog::debug("completely the creating of the region");

  // add the devices to the systems
  lighting_led.add_device(&led1);
  security_smoke.add_device(&dth11_1);
  security_smoke.add_device(&mq2_1);

  // add the systems to the region
  dormitory.add_system(&lighting_led);
  dormitory.add_system(&security_smoke);
  dormitory.set_security_system(&security_smoke);

  // create all threads for the region
  dormitory.region_thread(&dormitory);

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
