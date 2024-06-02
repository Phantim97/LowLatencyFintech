cmake_minimum_required(VERSION 3.26)
project(LowLatencyFintech)

set(CMAKE_CXX_STANDARD 20)

add_executable(LowLatencyFintech main.cpp
        basics.cpp
        basics.h
        RingBuffer.h
        mem_pool.h
        macros.h
        lock_free_q.h
        thread_utils.h
        time_utils.h
        Logger.h
)
