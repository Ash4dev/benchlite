/*
 * File: timer2.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-03
 * Description:
 * References:
 * https://www.youtube.com/watch?v=_Bo09H7EoHY
 * https://apeleg.com/blog/posts/2022/09/05/benchmarking-in-c-x86-and-x64/
 * https://alifara.codeberg.page/posts/timing-using-rdtsc/
 * https://blog.rutvora.com/p/accurately-timing-some-code/
 * NOTE: keep it working & simple
 */

// TODO: empty scoped timer benchmark

#include "utils/timer.hpp"
#include "utils/general.hpp"
#include <cassert>
#include <chrono>

#if USE_RDTSC
  #if defined(_MSC_VER) || defined(__MSC_VER)
    #include <intrin.h>
  #else
    #include <x86intrin.h>
  #endif
#endif

timer_type ScopedTimer::start_timer(){
  // __rdtsc is NOT inherently serialized: Out of Order execution
  #if USE_RDTSC
    _mm_lfence(); // wait for prev instr. to complete
    return __rdtsc();
  #else
    return std::chrono::steady_clock::now();
  #endif
}

timer_type ScopedTimer::end_timer(){
  #if USE_RDTSC
    // paritally serialized: wait for prior instr. to complete
    unsigned int aux; // cpu-id
    auto t = __rdtscp(&aux);
    _mm_lfence(); // block later loads from sneaking above
    return t;
  #else
    return std::chrono::steady_clock::now();
  #endif
}

std::chrono::nanoseconds ScopedTimer::elapsed_time_ns(duration_type duration){
  #if USE_RDTSC
   return std::chrono::nanoseconds(
       static_cast<long long>(duration * RDTSC_Calibration::get_CYCLE_TO_NS())
    );
  #else
   return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
  #endif
}

ScopedTimer::ScopedTimer(std::chrono::nanoseconds & duration)
: time_spent {duration} {
  start_point = start_timer();
}

ScopedTimer::~ScopedTimer(){
  end_point = end_timer();

  time_spent =
    elapsed_time_ns(end_point - start_point);
}

#if USE_RDTSC
  double RDTSC_Calibration::get_CYCLE_TO_NS() {
    // static local variable: static duration -> init only once
    double static CYCLE_TO_NS = calibrate();
    return CYCLE_TO_NS;
  }

  double RDTSC_Calibration::calibrate() {
    // warm-up the clock and branch-predictor
    auto cold_start_ns = std::chrono::steady_clock::now();
    long long x {};
    system_utils::do_not_optimize(x);
    while ((std::chrono::steady_clock::now() - cold_start_ns) <
        std::chrono::milliseconds(5)) {
      ScopedTimer::start_timer();
      x++;
    }
  
    // NOTE: maps CPU cycles to nanoseconds, NOT calculate timer overhead
    auto t0_ns = std::chrono::steady_clock::now();
    auto t0_cycles = ScopedTimer::start_timer();

    // spin for 100 ms / NUM_ITERATIONS can be too low / high
    x = 0;
    while ((std::chrono::steady_clock::now() - t0_ns) <
        std::chrono::milliseconds(1000)) {
      x++;
    }

    auto t1_ns = std::chrono::steady_clock::now();
    auto t1_cycles = ScopedTimer::end_timer();

    assert(t0_cycles < t1_cycles and t0_ns < t1_ns);

    return static_cast<double>(
       1.0 *
       std::chrono::duration_cast<std::chrono::nanoseconds>(t1_ns - t0_ns).count() /
       (t1_cycles-t0_cycles)
      );
  }

#endif
