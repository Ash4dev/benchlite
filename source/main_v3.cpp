/*
 * File: main.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-02
 * Description: 
 * NOTE: keep it working & simple
 */

#include "experiments.hpp"
#include <cstddef>

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
    // constant-clock unaffected by pinning, priority etc
    std::cout << "cycle-to-ns: " << RDTSC_Calibration::get_CYCLE_TO_NS() << "\n";
  #endif

  system_utils::pin_thread_to_core(4);
  system_utils::raise_thread_priority(); // TODO: bad operation suddenly. Fix it

  // NOTE: object construction works
  experiments::Experiment_Descriptor<experiments::GROWTH_EXPT> expt{
    "dynamic-sizing",
  {"IS_UNRESERVED", "IS_CONTAINER", "SIZE"}
  };

  // sample code for experiment 1
  for (int sz = 1; sz < 1e8; sz *= 10) {

    event_manager::Event::COMMON_PARAMS_DATA_TYPE config_params[] = {
      static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(experiments::ContainerType::VECTOR),
      static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(sz)
    };

    for (std::size_t i{}; i < 5; ++i){
      std::vector<int> v; system_utils::do_not_optimize(v); v.reserve(sz);
      if (i == 0) {
        system_utils::preapare_for_measurement([&](){
          for (std::size_t i{}; i < sz; ++i) {
            v.push_back(i);
          }
        });
        continue;
      }
      expt.measure(experiments::GROWTH_EXPT::RESERVED, config_params, [&](){
        for (std::size_t i{}; i < sz; ++i) {
          v.push_back(i);
        }
      });
      system_utils::do_not_optimize(v.data());
    }
  }

  for (int sz = 1; sz < 1e8; sz *= 10) {

    event_manager::Event::COMMON_PARAMS_DATA_TYPE config_params[] = {
      static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(experiments::ContainerType::VECTOR),
      static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(sz)
    };

    for (std::size_t i{}; i < 5; ++i){
      std::vector<int> v; system_utils::do_not_optimize(v.data());
      if (i == 0) {
        system_utils::preapare_for_measurement([&](){
          for (std::size_t i{}; i < sz; ++i) {
            v.push_back(i);
          }
        });
        continue;
      }
      expt.measure(experiments::GROWTH_EXPT::UNRESERVED, config_params, [&](){
        for (std::size_t i{}; i < sz; ++i) {
          v.push_back(i);
        }
      });
      system_utils::do_not_optimize(v.data());
    }
  }

  expt.flush_to_csv("run/growth2.csv");
  return 0;
}
