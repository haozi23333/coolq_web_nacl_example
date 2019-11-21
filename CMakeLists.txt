cmake_minimum_required(VERSION 3.6)
project(web_qq)


set(CMAKE_CXX_STANDARD 17)
set(NACL_SDK /Users/haozi/Downloads/nacl_sdk/pepper_55)
include_directories(${NACL_SDK}/include)
link_libraries(${NACL_SDK}/include)


add_executable(web_qq socket.cc)