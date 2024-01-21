#pragma once
#include <ctime>
#include <cstdio>
#include <type_traits>

using std::integral_constant;

enum log_level {
    _trace_ = 0,
    _debug_ = 1,
    _info_ = 2,
    _warn_ = 3,
    _error_ = 4
};

#ifndef LOG_LEVEL
#define LOG_LEVEL _info_
#endif

const char* log_level_to_string(log_level lvl) {
    switch (lvl) {
    case _trace_: return "[TRA]";
    case _debug_: return "[DBG]";
    case _info_: return "[INF]";
    case _warn_: return "[WAR]";
    case _error_: return "[ERR]";
    default: return "[???]";
    }
}

template<log_level lvl, class ...Args>
void logf(integral_constant<log_level, lvl>, const char* fmt, Args&& ...args) {
    if constexpr (lvl < LOG_LEVEL)
        return;
    char buf[1<<8];
    time_t t; time(&t);
    int off = sprintf(buf, "%s", ctime(&t));
    buf[off - 1] = ' ';
    sprintf(buf + off, "%s %s", log_level_to_string(lvl), fmt);
    fprintf(stderr, buf, forward<Args>(args)...);
}

template<log_level lvl, class ...Args>
void logf(const char* fmt, Args&& ...args) {
    logf(integral_constant<log_level, lvl>(), fmt, forward<Args>(args)...);
}

template<class ...Args>
void logf(const char* fmt, Args&& ...args) {
    logf<LOG_LEVEL>(fmt, forward<Args>(args)...);
}

// #define NO_ASSERT
#ifndef NO_ASSERT
// template<class ...Args>
// void assertf(bool cond, const char* fmt, Args&& ...args) {
//     if (cond)
//         return;
//     logf<_error_>(fmt, forward<Args>(args)...);
//     terminate();
//     assert(true);
// }
#define assertf(x) assert(x)
#else
#define assertf(x) (x)
#endif