#pragma once
#include <chrono>
#include "log.hpp"

namespace chrono = std::chrono;

struct timer {
    chrono::system_clock::time_point t_start;

    static auto now() { return chrono::system_clock::now(); }
    auto count() const { return chrono::duration_cast<chrono::milliseconds>(now() - t_start).count(); }
    timer() : t_start(now()) {};
    // ~timer() { logf<_info_>("%d\n", count()); }
};