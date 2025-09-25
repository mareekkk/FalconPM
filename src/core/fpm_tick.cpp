// src/core/fpm_tick.cpp
// FalconPM tick utility — implementation

#include "fpm_tick.h"
#include <chrono>

/**
 * Returns current tick in milliseconds using steady_clock
 * (monotonic, not affected by system clock changes).
 */
uint64_t fpm_gettick() {
    using namespace std::chrono;
    auto now = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(now).count();
}
