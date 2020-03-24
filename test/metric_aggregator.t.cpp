#include <chrono>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mtr/metrics.hpp"

using namespace ::testing;

TEST(metric_aggregator, macro_test) {
	for (int i = 0; i < 100; ++i) {
		METRICS_RECORD_BLOCK("test_metric");
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}

	const auto &aggregator = mtr::metric_aggregator::instance();
	EXPECT_EQ(aggregator.times_entered("test_metric"), 100);
	EXPECT_TRUE(aggregator.max<std::chrono::nanoseconds>("test_metric") >=
	            std::chrono::nanoseconds(10));

	EXPECT_EQ(aggregator.times_entered("i_don't_exist"), 0);
	EXPECT_EQ(aggregator.max<std::chrono::nanoseconds>("i_don't_exist"),
	          std::chrono::nanoseconds(0));
}

TEST(metric_aggregator, macro_nested_test) {
	const auto foo = []() {
		METRICS_RECORD_BLOCK("foo");
		for (int i = 0; i < 150; ++i) {
			METRICS_RECORD_BLOCK("foo_loop");
			std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
	};

	foo();

	const auto &aggregator = mtr::metric_aggregator::instance();

	EXPECT_EQ(aggregator.times_entered("foo"), 1);
	EXPECT_EQ(aggregator.times_entered("foo_loop"), 150);

	EXPECT_GE(aggregator.total<std::chrono::microseconds>("foo"),
	            std::chrono::microseconds(1));

	EXPECT_GE(aggregator.max<std::chrono::nanoseconds>("foo_loop"),
	            std::chrono::nanoseconds(10));
	EXPECT_GE(aggregator.min<std::chrono::nanoseconds>("foo_loop"),
	            std::chrono::nanoseconds(10));
	EXPECT_GE(aggregator.average<std::chrono::nanoseconds>("foo_loop"),
	            std::chrono::nanoseconds(10));

    const auto [min, max] = aggregator.min_max<std::chrono::nanoseconds>("foo_loop");
    EXPECT_EQ(min, aggregator.min<std::chrono::nanoseconds>("foo_loop"));
    EXPECT_EQ(max, aggregator.max<std::chrono::nanoseconds>("foo_loop"));
}
