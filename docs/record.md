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


## dormitory变量多线程设计
多个线程对dormitory对象的引用，而且没有多线程安全设计会导致，冲突。


## thread start segmentation fault
```sh
cuier@master:~/github.com/dormitoryIOT-edge$ ./app  
[2023-04-30 12:42:58.059] [thread 188812] [debug] security_thread start
[2023-04-30 12:42:58.059] [thread 188813] [debug] handle_message_thread start
Segmentation fault (core dumped)
cuier@master:~/github.com/dormitoryIOT-edge$ ./app 
[2023-04-30 12:43:04.157] [thread 188931] [debug] security_thread start
Segmentation fault (core dumped)
cuier@master:~/github.com/dormitoryIOT-edge$ ./app 
[2023-04-30 12:43:06.378] [thread 188973] [debug] handle_message_thread start
[2023-04-30 12:43:06.378] [thread 188972] [debug] security_thread start
[2023-04-30 12:43:06.378] [thread 188974] [debug] control_panel_thread start
Segmentation fault (core dumped)
cuier@master:~/github.com/dormitoryIOT-edge$ ./app 
[2023-04-30 12:43:08.561] [thread 188981] [debug] handle_message_thread start
[2023-04-30 12:43:08.561] [thread 188980] [debug] security_thread start
Segmentation fault (core dumped)
cuier@master:~/github.com/dormitoryIOT-edge$ ./app 
[2023-04-30 12:43:11.049] [thread 189016] [debug] handle_message_thread start
[2023-04-30 12:43:11.049] [thread 189015] [debug] security_thread start
[2023-04-30 12:43:11.049] [thread 189014] [debug] main thread is running
Segmentation fault (core dumped)
cuier@master:~/github.com/dormitoryIOT-edge$ 
```
