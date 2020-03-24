#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mtr/metrics.hpp"

#include <array>
#include <chrono>
#include <numeric>

using namespace ::testing;

TEST(block_recording, update_test) {
    mtr::block_recording block;

    for (std::size_t i = 0; i < mtr::MAX_RECORDINGS; ++i) {
        block.update(std::chrono::nanoseconds(i));
    }
    
    std::array<std::chrono::nanoseconds, mtr::MAX_RECORDINGS> expected;
    std::iota(expected.begin(), expected.end(), std::chrono::nanoseconds(0));
    EXPECT_THAT(expected, ElementsAreArray(block.begin(), block.end()));

    for (std::size_t i = mtr::MAX_RECORDINGS + 1; i < mtr::MAX_RECORDINGS + 10; ++i) {
        block.update(std::chrono::nanoseconds(i));
    }

    /* Test the wrapping around */
    std::iota(expected.begin(), expected.begin() + 9, std::chrono::nanoseconds(mtr::MAX_RECORDINGS + 1));
    EXPECT_THAT(expected, ElementsAreArray(block.begin(), block.end()));
}

TEST(block_recording, few_recordings) {
    mtr::block_recording block;
    for (std::size_t i = 0; i < 10; ++i) {
        block.update(std::chrono::nanoseconds(i));
    }
    
    EXPECT_EQ(block.end(), block.begin() + 10);
}
