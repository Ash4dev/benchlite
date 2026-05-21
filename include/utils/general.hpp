/*
 * File: general.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-12
 * Description: 
 * NOTE: keep it working & simple
 */

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <thread>

// https://www.geeksforgeeks.org/operating-systems/how-to-detect-operating-system-through-a-c-program/
#if defined(__linux) || defined(__linux__)
  #define LINUX 1
  #include <sched.h>
  #include <pthread.h>
#elif defined(_WIN32) || defined(_WIN64)
  #define WINDOWS 1
  #include <intrin.h>
  #pragma optimize("", off)
  #include <windows.h>
#endif
namespace system_utils {

  template<typename T>
  void inline do_not_optimize(T const& value) {
    #if defined (_GNU_SOURCE)
      asm volatile("" : : "r,m"(value) : "memory");
    #elif defined (WINDOWS)
      [[maybe_unused]] char const volatile *dummy = reinterpret_cast<char const volatile*>(&value);
      (void)dummy;
      _ReadWriteBarrier();
    #endif
  }

  template<typename Fn>
  void inline preapare_for_measurement(Fn &&fn){
    long long x {};
    system_utils::do_not_optimize(x);
    auto begin_time = std::chrono::steady_clock::now();
    while ((std::chrono::steady_clock::now() - begin_time) <
        std::chrono::milliseconds(5)) {
      fn();
      x++;
    }
  }

  // current configuration based on personal hardware TODO: generalization

  // responsiblity of user to choose threads NOT on the same core

  // OS core preferrence
    // cat /sys/devices/system/cpu/isolated
      // Empty
    // taskset -cp 1 => affinity of PID 1
      // 0 to 11 on personal

  uint32_t inline const CORE_CNT {std::thread::hardware_concurrency()};

  // Next best thing to isolation -> raise priority
  void inline raise_thread_priority() {
    // requires ROOT ACCESS

    #if defined(LINUX)
      // https://man7.org/linux/man-pages/man7/sched.7.html
      // each thread: has scheduler policy and priority

      // SCHED_OTHER, SCHED_IDLE, SCHED_BATCH: normal thread policies
        // NOT affected by thread priority
      
      // SCHED_FIFO, SCHED_RR: real-time policies affected by priority
      sched_param sched_params {};

      // benchmarking: go with FIFO not RR
        // RR: thread time-sliced by OS for other
        // TSC is constant: keeps going on
      // timer goes on RDTSC based constant clock

      // TODO: needs to be fine-tuned
      sched_params.sched_priority = 70; // low-high: 1-99

      int res {};
      if (0 != (res = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched_params))) {
        throw std::runtime_error(std::strerror(res));
      }
    #elif defined(WINDOWS)
      // https://scorpiosoftware.net/2023/07/14/thread-priorities-in-windows/
      // process level: REALTIME_PRIORITY_CLASS
      // thread level: THREAD_PRIORITY_HIGHEST(pre-empt only lower, safer),
      // THREAD_PRIORITY_TIME_CRITICAL (pre-empt all else)

      if (0 == SetThreadPriority(reinterpret_cast<HANDLE>(GetCurrentThread()), THREAD_PRIORITY_HIGHEST)) {
        throw std::runtime_error("Failed to set thread priority\n");
      }
    #endif
  }

  void inline pin_thread_to_core(uint8_t logical_core_id) {
    // https://towardsdev.com/cpp-cpu-affinity-core-pinning-ba6749faaa65
    // https://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
    // https://man7.org/linux/man-pages/man3/CPU_SET.3.html

    // ties current thread to that core, does NOT prevent core to others
      // not even its children threads
    // reduce current thread migrations, NOT context switches by preemption
    // true benefit: p99, jitter

    // true core isolation: modify bootloader config / kernel boot params
    // parameters to enable: isolcpu, nohz_full, rcu_nocbs, irqaffinity

    if (logical_core_id >= CORE_CNT) {
      // runtime_error can be captured in try-catch block
      throw std::invalid_argument("Invalid number of cores\n");
    }

    int res{};
    #if defined(LINUX)
      cpu_set_t cpuset{}; CPU_ZERO(&cpuset); CPU_SET(logical_core_id, &cpuset); // bit-mask like impl
      if (0 != (res = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset))) {
        throw std::runtime_error(std::strerror(res));
      }
    #elif defined(WINDOWS)
      DWORD_PTR mask {static_cast<DWORD_PTR>(1) << core_id};
      if (0 == (res = SetThreadAffinityMask(GetCurrentThread(), mask))){
        throw std::runtime_error(std::strerror(res));
     }
    #endif
  }
}
