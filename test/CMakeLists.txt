add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

set(TESTS block_recording.t.cpp metric_aggregator.t.cpp)

add_executable(cpp-metrics-test ${TESTS})
target_compile_options(cpp-metrics-test PUBLIC ${CPP-METRICS_CXX_FLAGS})

target_link_libraries(cpp-metrics-test PUBLIC cpp-metrics)
target_link_libraries(cpp-metrics-test PUBLIC gtest gtest_main gmock gmock_main)

add_test(test cpp-metrics-test)
