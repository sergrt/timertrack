#pragma once
#include <chrono>

struct Record {
    int id_;
    enum Status {
        Started = 0,
        Finished = 1,
        Interrupted = 2
    };
    Status status_;
    int category_;
    std::chrono::time_point<std::chrono::system_clock> startTime_;
    std::chrono::milliseconds plannedTime_;
    std::chrono::milliseconds passedTime_;
};

