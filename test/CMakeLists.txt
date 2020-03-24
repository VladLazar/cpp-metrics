add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

set(TESTS block_recording.t.cpp metric_aggregator.t.cpp)

add_executable(cpp-metrics-test ${TESTS})
target_link_libraries(cpp-metrics-test PUBLIC cpp-metrics)
target_link_libraries(cpp-metrics-test PUBLIC gtest gtest_main gmock gmock_main)

# Enable collection of metrics
add_compile_definitions(COLLECT_METRICS=1)

add_test(test cpp-metrics-test)
