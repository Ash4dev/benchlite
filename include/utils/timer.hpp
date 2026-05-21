/*
 * File: timer.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-02
 * Description:
 * NOTE: keep it working & simple
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <cstdint>

// https://stackoverflow.com/a/77675880

// defined supports boolean expression -> only valid in #if / #elif
// must know beforehand actual macro names for supported compilers
#if defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
  #define USE_RDTSC 1
#else
  #define USE_RDTSC 0
#endif

#if USE_RDTSC
  using timer_type = uint64_t;
  using duration_type = uint64_t;

  struct RDTSC_Calibration{
    private:
      static double calibrate(); // restrict visibility of calibrate
    public:
      static double get_CYCLE_TO_NS(); // no recall with static local variable
  };
#else
  // steady_clock: hardware ensures monotonicity
  // high_performance_clock not used as alias for system_clock
  using timer_type = std::chrono::steady_clock::time_point;
  using duration_type = std::chrono::steady_clock::duration;
#endif


// RAII-based class: start and end time made private and order misuse avoided
class ScopedTimer {
// NOTE: singular responsibility of measuring time of an operation

private:

  #if USE_RDTSC
    // RDTSC_Calibration uses start_timer and end_timer
    friend struct RDTSC_Calibration;
  #endif

  timer_type start_point;
  timer_type end_point;
  std::chrono::nanoseconds &time_spent;

  // static member fns -> do not operate on object state aka stateless fns
  timer_type static start_timer();
  timer_type static end_timer();
  std::chrono::nanoseconds static elapsed_time_ns(duration_type);

public:
  ScopedTimer() = delete;
  // events tracked -> pass event.duration as reference
  explicit ScopedTimer(std::chrono::nanoseconds &);
  ~ScopedTimer();

  ScopedTimer(ScopedTimer const &) = delete;
  ScopedTimer &operator=(ScopedTimer const &) = delete;

  ScopedTimer(ScopedTimer &&) = delete;
  ScopedTimer &operator=(ScopedTimer &&) = delete;
};

#endif
