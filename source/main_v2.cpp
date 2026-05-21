/*
 * File: main.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-02
 * Description: 
 * NOTE: keep it working & simple
 */

#include "timer.hpp"
#include "experiments.hpp"
#include "event_manager.hpp"
#include "general.hpp"

#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>

namespace experiments {
  // https://stackoverflow.com/a/14989325
  // scoped within and no implicit int conversion
  enum class ALLOCATION_EXPT : std::uint8_t {RESERVED, UNRESERVED, INTERNAL_COUNT};
  enum class LOCALITY_EXPT : std::uint8_t {SMALL_OBJECT, LARGE_OBJECT, INTERNAL_COUNT};
  enum class GROWTH_EXPT : std::uint8_t {RESERVED, UNRESERVED, INTERNAL_COUNT};
  enum class ACCESS_EXPT : std::uint8_t {RANDOM_KEY, HOT_KEY, INTERNAL_COUNT};
  enum class ORDER_EXPT: std::uint8_t {RANDOM_KEY, HOT_KEY, INTERNAL_COUNT};
  
  enum class ContainerType: std::uint8_t {VECTOR, LIST, DEQUE,
                                          UNORDERED_SET, INTERNAL_COUNT};
}

int main(){
  #if USE_RDTSC
    std::cout << "cycle-to-ns: " << RDTSC_Calibration::get_CYCLE_TO_NS() << "\n";
  #endif

  system_utils::pin_thread_to_core(4);
  system_utils::raise_thread_priority();

  experiments::Experiment_Descriptor<experiments::GROWTH_EXPT> expt{
    "dynamic-sizing",
  {"IS_UNRESERVED", "IS_CONTAINER", "SIZE"}
  };

  // sample code for experiment 1
  for (int sz = 1; sz < 1e8; sz *= 10) {
    std::vector<int> v;
    system_utils::do_not_optimize(v);

    v.reserve(sz);
    event_manager::Event e;
    e.param_cnt = expt.param_cnt;
    e.params[0] = static_cast<std::size_t>(experiments::GROWTH_EXPT::RESERVED);
    e.params[1] = static_cast<std::size_t>(experiments::ContainerType::VECTOR);
    e.params[2] = sz;
    {
      ScopedTimer t1(e.time_elapsed);
      // NOTE: do I need to average the measurement for each size too?
      for (int i = 0; i < sz; ++i){
        v.push_back(i);
      }
   }
    e.time_elapsed /= sz;
    expt.tracker.add(e);
  }

  for (int sz = 1; sz < 1e8; sz *= 10) {
    std::vector<int> v;
    system_utils::do_not_optimize(v);

    event_manager::Event e;
    e.param_cnt = expt.param_cnt;
    e.params[0] = static_cast<std::size_t>(experiments::GROWTH_EXPT::UNRESERVED);
    e.params[1] = static_cast<std::size_t>(experiments::ContainerType::VECTOR);
    e.params[2] = sz;
    {
      ScopedTimer t1(e.time_elapsed);
      // NOTE: do I need to average the measurement for each size too?
      for (int i = 0; i < sz; ++i){
        v.push_back(i);
      }
   }

    e.time_elapsed /= sz;
    expt.tracker.add(e);
  }

  std::cout << "number of operations: " << expt.tracker.size() << "\n";

  for (auto const &op_track: expt.tracker){
    std::cout << "number of events: " << op_track.size() << "\n";
    for (auto const &event: op_track){
      for (std::size_t i {}; i < event.param_cnt; ++i){
        std::cout << event.params[i] << " ";
      }
      std::cout << static_cast<long long>(event.time_elapsed.count()) << "\n";
    }
  }

  event_manager::flush_to_csv("run/growth.csv", 
      expt.param_cnt,
      expt.param_labels,
      expt.tracker);
  return 0;
}
