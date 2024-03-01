cmake_minimum_required(VERSION 3.27)
project(LowLatencyFintech)

set(CMAKE_CXX_STANDARD 20)

add_executable(LowLatencyFintech main.cpp
        basics.cpp
        basics.h
        RingBuffer.h
)
