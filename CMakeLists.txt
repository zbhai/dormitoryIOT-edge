# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (app)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 math 子目录
add_subdirectory(pkg/message)

# 添加对gdb的支持
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


# 指定生成目标 
add_executable(app ${DIR_SRCS})

# 添加链接库
target_link_libraries(app 
message
paho-mqtt3a
paho-mqtt3as
paho-mqtt3c
paho-mqtt3cs
paho-mqttpp3
)