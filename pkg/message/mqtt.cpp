// async_subscribe.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// This application is an MQTT subscriber using the C++ asynchronous client
// interface, employing callbacks to receive messages and status updates.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker.
//  - Subscribing to a topic
//  - Receiving messages through the callback API
//  - Receiving network disconnect updates and attempting manual reconnects.
//  - Using a "clean session" and manually re-subscribing to topics on
//    reconnect.
//

/*******************************************************************************
 * Copyright (c) 2013-2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include "mqtt/async_client.h"
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

#include<unistd.h>      //linux用户级
#include<time.h>
#include<utime.h>


#include "dormitory.hpp"
#include "message.hpp"
#include "mqtt.hpp"
#include "split.hpp"
#include "MPMCQueue.h"

using namespace std;

const std::string SERVER_ADDRESS("localhost:1883");

const std::string CLIENT_ID{"dormitoryIOT"};
const std::string TOPIC{"dormitoryIOT/test"};
const std::string REC_DEVICE_TOPIC{"$hw/events/device/+/state/update"};
const std::string REC_DEVICE_TOPIC_THIRDY{"$ke/events/device/+/data/update"};
const std::string SEN_DEVICE_TOPIC{"$hw/events/device/+/twin/update/delta"};
const std::string SEN_CLOUD_TOPIC{"$SYS/dis/upload_records"};

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

extern rigtorp::MPMCQueue<message> lighting_queue;
extern rigtorp::MPMCQueue<message> security_queue;
extern rigtorp::MPMCQueue<message> general_queue;
extern rigtorp::MPMCQueue<message> downstream_queue;
extern rigtorp::MPMCQueue<message> downstream_security;

const auto TIMEOUT = std::chrono::seconds(10);



const char* PAYLOAD1 = "Hello World!";
const char* PAYLOAD2 = "Hi there!";
const char* PAYLOAD3 = "Is anyone listening?";
const char* PAYLOAD4 = "Someone is always listening.";

const char* LWT_PAYLOAD = "Last will and testament.";

void *mqtt_pub_thread(void *);

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener {
  std::string name_;

  void on_failure(const mqtt::token &tok) override {
    std::cout << name_ << " failure";
    if (tok.get_message_id() != 0)
      std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    std::cout << std::endl;
  }

  void on_success(const mqtt::token &tok) override {
    std::cout << name_ << " success";
    if (tok.get_message_id() != 0)
      std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
    auto top = tok.get_topics();
    if (top && !top->empty())
      std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
    std::cout << std::endl;
  }

public:
  action_listener(const std::string &name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A base action listener.
 */
class action_listener_dlv : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override {
		cout << "\tListener failure for token: "
			<< tok.get_message_id() << endl;
	}

	void on_success(const mqtt::token& tok) override {
		cout << "\tListener success for token: "
			<< tok.get_message_id() << endl;
	}
};

/**
 * A derived action listener for publish events.
 */
class delivery_action_listener : public action_listener_dlv
{
	atomic<bool> done_;

	void on_failure(const mqtt::token& tok) override {
		action_listener_dlv::on_failure(tok);
		done_ = true;
	}

	void on_success(const mqtt::token& tok) override {
		action_listener_dlv::on_success(tok);
		done_ = true;
	}

public:
	delivery_action_listener() : done_(false) {}
	bool is_done() const { return done_; }
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
                 public virtual mqtt::iaction_listener

{
  // Counter for the number of connection retries
  int nretry_;
  // The MQTT client
  mqtt::async_client &cli_;
  // Options to use if we need to reconnect
  mqtt::connect_options &connOpts_;
  // An action listener to display the result of actions.
  action_listener subListener_;

  // This deomonstrates manually reconnecting to the broker by calling
  // connect() again. This is a possibility for an application that keeps
  // a copy of it's original connect_options, or if the app wants to
  // reconnect with different options.
  // Another way this can be done manually, if using the same options, is
  // to just call the async_client::reconnect() method.
  void reconnect() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    try {
      cli_.connect(connOpts_, nullptr, *this);
    } catch (const mqtt::exception &exc) {
      std::cerr << "Error: " << exc.what() << std::endl;
      exit(1);
    }
  }

  // Re-connection failure
  void on_failure(const mqtt::token &tok) override {
    std::cout << "Connection attempt failed" << std::endl;
    if (++nretry_ > N_RETRY_ATTEMPTS)
      exit(1);
    reconnect();
  }

  // (Re)connection success
  // Either this or connected() can be used for callbacks.
  void on_success(const mqtt::token &tok) override {

  }

  // (Re)connection success
  void connected(const std::string &cause) override {
    std::cout << "\nConnection success" << std::endl;
    std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
              << "\tfor client " << CLIENT_ID << " using QoS" << QOS << "\n"
              << "\nPress Q<Enter> to quit\n"
              << std::endl;

    cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
    cli_.subscribe(REC_DEVICE_TOPIC, QOS, nullptr, subListener_);
    cli_.subscribe(REC_DEVICE_TOPIC_THIRDY, QOS, nullptr, subListener_);
  }

  // Callback for when the connection is lost.
  // This will initiate the attempt to manually reconnect.
  void connection_lost(const std::string &cause) override {
    std::cout << "\nConnection lost" << std::endl;
    if (!cause.empty())
      std::cout << "\tcause: " << cause << std::endl;

    std::cout << "Reconnecting..." << std::endl;
    nretry_ = 0;
    reconnect();
  }

  // Callback for when a message arrives.
  void message_arrived(mqtt::const_message_ptr msg) override {
    std::cout << "Message arrived" << std::endl;
    std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
    std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;

    // prehandle the message and pass to the message dispatcher
    std::string topic = msg->get_topic();
    std::string data = msg->get_payload_str();
    // auto topic_elem = split(topic, '/');
    // std::string device = topic_elem[3];
    // int index = device.find("lighting");
    int index = topic.find("lighting");
    if (index > 0) {
      bool ret = lighting_queue.try_emplace(data, topic);
      if(ret != true)
      {
        spdlog::debug("message_arrived lighting queue try emplace error!\n\r");
      }
      else
      {
        spdlog::debug("message_arrived lighting queue try emplace success!\n\r");
      }
    }
    index = topic.find("security");
    if (index > 0) {
      security_queue.emplace(data, topic);
    }
    index = topic.find("node-led-dth11-buzzer");
    if(index > 0)
    {
      bool ret = general_queue.try_emplace(data, topic);
      if(ret != true)
      {
        spdlog::debug("message_arrived general queue try emplace error!\n\r");
      }
      else
      {
        spdlog::debug("message_arrived general queue try emplace success!\n\r");
      }
    }
    index = topic.find("bh1750");
    if(index > 0)
    {
      bool ret = general_queue.try_emplace(data, topic);
      if(ret != true)
      {
        spdlog::debug("message_arrived general queue try emplace error!\n\r");
      }
      else
      {
        spdlog::debug("message_arrived general queue try emplace success!\n\r");
      }
    }
    index = topic.find("mq2");
    {
      bool ret = general_queue.try_emplace(data, topic);
      if(ret != true)
      {
        spdlog::debug("message_arrived general queue try emplace error!\n\r");
      }
      else
      {
        spdlog::debug("message_arrived general queue try emplace success!\n\r");
      }
    }
  }

  void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
  callback(mqtt::async_client &cli, mqtt::connect_options &connOpts)
      : nretry_(0), cli_(cli), connOpts_(connOpts),
        subListener_("Subscription") {}
};

/////////////////////////////////////////////////////////////////////////////

void *mqtt_thread(void *arg) {
  // A subscriber often wants the server to remember its messages when its
  // disconnected. In that case, it needs a unique ClientID and a
  // non-clean session.

  mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

  mqtt::connect_options connOpts;
  connOpts.set_clean_session(false);

  // Install the callback(s) before connecting.
  callback cb(client, connOpts);
  client.set_callback(cb);

  // Start the connection.
  // When completed, the callback will subscribe to topic.

  try {
    std::cout << "Connecting to the MQTT server..." << std::flush;
    mqtt::token_ptr conntok = client.connect(connOpts, nullptr, cb);
    std::cout << "waiting for the connect complete..." << std::flush;
    //conntok->wait();
    cout << "  ...OK" << endl;
  } catch (const mqtt::exception &exc) {
    std::cerr << "\nERROR: Unable to connect to MQTT server: '"
              << SERVER_ADDRESS << "'" << exc << std::endl;
    return nullptr;
  }

    sleep(5);

  while(true)
  {
    // loop for the checkout the message queue
    if(!downstream_security.empty())
    { 
      message m;
      downstream_security.pop(m);
      spdlog::debug("publish message topic {}, message data{}", m.get_topic(), m.get_data());
      mqtt::message_ptr pubmsg = mqtt::make_message(m.get_topic(), m.get_data());
      pubmsg->set_qos(1);
      client.publish(pubmsg)->wait_for(TIMEOUT);
      spdlog::debug("publish message success!\n\r");
    }
    if(!downstream_queue.empty())
    {
      message m;
      downstream_queue.pop(m);
      spdlog::debug("publish message topic {}, message data{}", m.get_topic(), m.get_data());
      mqtt::message_ptr pubmsg = mqtt::make_message(m.get_topic(), m.get_data());
      pubmsg->set_qos(0);
      client.publish(pubmsg)->wait_for(TIMEOUT);
      spdlog::debug("publish message success!\n\r");
    }
    sleep(1);
  }

  // Disconnect
  try {
    std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
    client.disconnect()->wait();
    std::cout << "OK" << std::endl;
  } catch (const mqtt::exception &exc) {
    std::cerr << exc << std::endl;
    return nullptr;
  }

  return 0;
}

