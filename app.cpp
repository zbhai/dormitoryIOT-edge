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

void *data_center_thread(void *arg);
void *publish_thread(void *arg);

/////////////////////////////////////////////////////////////////////////////

void *mqtt_thread(void *arg) {

  // setting the log
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] %v");
  // log the information
  spdlog::debug("start the mqtt client");

  // create the thread to handle the message
  pthread_t data_center_thread_id;
  pthread_create(&data_center_thread_id, NULL, data_center_thread, NULL);
  // create the thread to publish the message
  pthread_t publish_thread_id;
  pthread_create(&publish_thread_id, NULL, publish_thread, NULL);

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
      sub_topics.push(data, topic);
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
    return 1;
  }

  return 0;
}

// the thread handle the message from the sub_topics queue
void *data_center_thread(void *arg) {
  while (1) {
    if (sub_topics.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    message m = sub_topics.pop();
    std::string data = m.get_data();
    std::string topic = m.get_topic();
    cout << "data: " << data << endl;
  }
}

void *publish_thread(void *arg) {
  while (1) {
    if (pub_topics.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    message m = pub_topics.pop();
    std::string data = m.get_data();
    std::string topic = m.get_topic();
    cout << "data: " << data << endl;
  }
}

// the thread create a reagion and handle the message from the reagion
void *main_thread(void *arg) {
  // create the region
  dormitoryIOT dormitory("dormitory614", "dormitory614_id");
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
  dormitory.add_system(&lighting_led);
  dormitory.add_system(&security_smoke);

  // create all threads for the region
  dormitory.region_thread(nullptr);

  // receive message from the sub_topics queue and handle its
  while (1) {
    if (sub_topics.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    message m = sub_topics.pop();
    std::string data = m.get_data();
    std::string topic = m.get_topic();
    cout << "data: " << data << endl;
    // handle the message
    dormitory.handle_message(m);
  }
}
