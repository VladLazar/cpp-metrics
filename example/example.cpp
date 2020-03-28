#include "mtr/metrics.hpp"

#include <cassert>
#include <chrono>
#include <vector>
#include <iostream>
#include <thread>

std::vector<int> foo() {
    METRICS_RECORD_BLOCK("foo");
    std::vector<int> vals;

    for (int i = 0; i < 100; ++i) {
        METRICS_RECORD_BLOCK("foo_loop");

        vals.push_back(i);
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }

    return vals;
}

int main() {
    (void) foo();

    const auto &aggregator = mtr::metric_aggregator::instance();

    assert(aggregator.times_entered("foo") ==  1);
    assert(aggregator.times_entered("foo_loop") == 100);

    assert(aggregator.total<std::chrono::microseconds>("foo") > std::chrono::microseconds(1));
    
    assert(aggregator.max<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));
    assert(aggregator.min<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));
    assert(aggregator.average<std::chrono::nanoseconds>("foo_loop") > std::chrono::nanoseconds(10));

    aggregator.dump_metrics<std::chrono::nanoseconds>("foo_loop", std::cout);
    /*
     * Outputs the following to standard output
     *
     * foo_loop metrics:
     *     Entered: 100
     *     Total: 5355843ns
     *     Average: 53558ns
     *     Min: 26478ns
     *     Max: 85410ns
     */

    return 0;
}
