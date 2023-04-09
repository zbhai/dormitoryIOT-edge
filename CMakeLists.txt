cmake_minimum_required(VERSION 2.8.3)
project(app)
add_compile_options(-std=c++11)

include_directories(
  include
)

add_executable(app app.cpp)

target_link_libraries(app
paho-mqtt3a
paho-mqtt3as
paho-mqtt3c
paho-mqtt3cs
paho-mqttpp3
)

