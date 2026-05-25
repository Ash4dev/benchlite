/*
 * File: 3-growth.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-11
 * Description: 
 * NOTE: keep it working & simple
 */

#include "utils/experiments.hpp"

namespace growth_expt {
  enum class GROWTH_EXPT : std::uint8_t {RESERVED, UNRESERVED, INTERNAL_COUNT};
  enum class ContainerType: std::uint8_t {VECTOR, INTERNAL_COUNT};
}

inline void run_vector_growth_experiment(std::filesystem::path const& relative_file_path) {
  constexpr std::size_t WARMUP_RUNS = 3;
  constexpr std::size_t MEASURED_RUNS = 20;

  experiments::Experiment_Descriptor<growth_expt::GROWTH_EXPT> expt{
    "dynamic-sizing",
    {"IS_UNRESERVED", "IS_CONTAINER", "SIZE"}
  };

  for (int sz = 1; sz < 1e8; sz *= 10) {
    event_manager::Event::COMMON_PARAMS_DATA_TYPE params[] = {
        static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(
            growth_expt::ContainerType::VECTOR
        ),
        static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(sz)
    };
    
    // RESERVED - one call, no manual warmup
    expt.measure_after_warmup(
      growth_expt::GROWTH_EXPT::RESERVED,
      params,
      [sz]{
          std::vector<int> v;
          v.reserve(sz);
          for (size_t j{}; j < sz; ++j) v.push_back(j);
          system_utils::do_not_optimize(v.data());
      }
    );
    
    // UNRESERVED - also one call
    expt.measure_after_warmup(
      growth_expt::GROWTH_EXPT::UNRESERVED,
      params,
      [sz]{
          std::vector<int> v;
          for (size_t j{}; j < sz; ++j) v.push_back(j);
          system_utils::do_not_optimize(v.data());
      }
    );
  }

  expt.flush_to_csv(relative_file_path);
}
