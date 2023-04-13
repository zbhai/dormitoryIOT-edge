# main function

## 1. mqtt module:
- receive data from mapper
  mqtt topic: 
- send data to mapper
  mqtt topic:
- send data to cloud
  mqtt topic:

## wild pointer 
    // // the time control panel is based on groups
    // auto iter = lighting_groups.find("dormitory");
    // auto devices = iter->second;
    // for (auto device : devices) {
    //   if (hour >= 23 || hour <= 6) {
    //     // turn on the light
    //     led *lighting = dynamic_cast<led *>(device);
    //     auto m = lighting->set_desired_status("on");
    //     dormitory->push_message(m.get_data(), m.get_topic());
    //   } else {
    //     // turn off the light
    //     led *lighting = dynamic_cast<led *>(device);
    //     auto m = lighting->set_desired_status("off");
    //     dormitory->push_message(m.get_data(), m.get_topic());
    //   }
    // }
    // turn on/off the light based on the motion sensor
    // now the motion sensor is empty

# project setup
- region setup
  - add general systems
    - lighting system
  - set security system
- system setup
  - lighting system
    - add devices
    - add groups
- device setup
  - led
  - dth11
  - mq2
- mqtt setup