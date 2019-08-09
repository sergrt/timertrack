// TimerTrack_UnitTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <gtest/gtest.h>

TEST(Foo, FooFirstTest) {
    EXPECT_EQ(1, 1);
}

int main() {
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
}
