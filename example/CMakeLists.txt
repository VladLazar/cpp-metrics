add_executable(example example.cpp)
target_link_libraries(example cpp-metrics)
target_compile_options(example PUBLIC ${CPP-METRICS_CXX_FLAGS})
