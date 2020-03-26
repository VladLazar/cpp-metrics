# cpp-metrics

## Description
Simple, header only library for the collection of metrics.

### Example
```cpp
#include "mtr/metrics.cpp"

#include <chrono>
#include <thread>

void foo() {
    METRICS_RECORD_BLOCK("foo");
    for (int i = 0; i <= 100; ++i) {
        METRICS_RECORD_BLOCK("foo_loop");
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
}

int main() {
    foo();

    const auto &aggregator = mtr::metric_aggregator::instance();

    assert(aggregator.times_entered("foo") ==  1);
    assert(aggregator.times_entered("foo_loop") == 100);

    assert(aggregator.total<std::chrono::microseconds>("foo") > std::chrono::microseconds(1));
    
    assert(aggregator.max<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));
    assert(aggregator.min<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));
    assert(aggregator.average<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));

    return 0;
}
```
