#ifndef FRAG_DEBUG_H_
#define FRAG_DEBUG_H_

#include <chrono>

#define DEBUG_TIME_DECLARE(name) \
     std::chrono::time_point<std::chrono::high_resolution_clock> start_ ## name; \
     std::chrono::time_point<std::chrono::high_resolution_clock> end_ ## name;

#define DEBUG_TIME_START(name) \
    if (debug_time) { \
        start_##name = std::chrono::high_resolution_clock::now(); \
    }

#define DEBUG_TIME_END(name) \
    if (debug_time) { \
        end_##name = std::chrono::high_resolution_clock::now(); \
        std::cout << "debug-timer " # name << " " << std::chrono::duration<double, std::milli>(end_##name - start_##name).count() << "ms" << std::endl; \
    }

#endif
