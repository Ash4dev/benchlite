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
#include <iostream>
#include <vector>

// user defines their experiments here
namespace experiments {
  // https://www.learncpp.com/cpp-tutorial/scoped-enumerations-enum-classes/
  // no implicit int conversion & scoped within
  // https://stackoverflow.com/a/14989325

  // ALLOCATION_EXPT -> type and ALLOCATION_EXPT::RESERVED -> value of that type
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
    std::cout << "begin RDTSC calibration\n";
    auto t = RDTSC_Calibration::get_CYCLE_TO_NS();
    std::cout << t << std::endl;
    std::cout << "end RDTSC calibration\n";
  #endif

  event_manager::Event e;
  e.param_cnt = 1;
  event_manager::Tracker<experiments::GROWTH_EXPT> tk;

  // sample code for experiment 1
  for (int sz = 1; sz < 1e8; sz *= 10) {
    std::vector<int> v;
    system_utils::do_not_optimize(v);
    v.reserve(sz);
    e.params[0] = sz;
    {
      ScopedTimer t1(e.time_elapsed);
      for (int i = 0; i < sz; ++i){
        v.push_back(i);
      }
   }
    // e.container = experiments::ContainerType::VECTOR;
    e.time_elapsed /= sz;
    tk.add(experiments::GROWTH_EXPT::RESERVED, e);
  }

  std::cout << "number of operations: " << tk.size() << "\n";
  // tracker is specific to each experiment
  for (auto const &op_track: tk){
    std::cout << "number of events: " << op_track.size() << "\n";
    for (auto const &event: op_track){
      for (auto const &ele: event.params){
        std::cout << ele << " ";
      }
      std::cout << static_cast<long long>(event.time_elapsed.count()) << "\n";
    }
  }

  event_manager::flush_to_csv("run/lol.csv", tk);

  experiments::Experiment_Descriptor<experiments::GROWTH_EXPT> t1;

  return 0;
}
