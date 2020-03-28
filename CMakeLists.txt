cmake_minimum_required(VERSION 3.15)

project(CPP-METRICS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic")

option(CPP-METRICS_BUILD_TEST_AND_EXAMPLE "Build tests" OFF)

add_library(cpp-metrics INTERFACE)
target_include_directories(cpp-metrics INTERFACE include/)

if(CPP-METRICS_BUILD_TEST_AND_EXAMPLE)
    # Enable collection of metrics
    add_compile_definitions(COLLECT_METRICS=1)

    enable_testing()
    add_subdirectory(test/)
    add_subdirectory(example/)
endif()
