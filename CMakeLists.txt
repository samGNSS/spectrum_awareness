cmake_minimum_required(VERSION 2.6)
project(spectrum_awareness)

add_executable(spectrum_awareness main.cpp)

install(TARGETS spectrum_awareness RUNTIME DESTINATION bin)
