#include <gmock/gmock-matchers.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mtr/metrics.hpp"

#include <array>
#include <chrono>
#include <numeric>

using namespace ::testing;

TEST(block_recording, update_test) {
    mtr::block_recording block;
    block.update(std::chrono::nanoseconds(10));
    block.update(std::chrono::nanoseconds(20));
    block.update(std::chrono::nanoseconds(30));

    EXPECT_THAT(block.times_entered(), 3);
    EXPECT_THAT(block.total(), std::chrono::nanoseconds(60));
    EXPECT_THAT(block.min(), std::chrono::nanoseconds(10));
    EXPECT_THAT(block.max(), std::chrono::nanoseconds(30));
}
