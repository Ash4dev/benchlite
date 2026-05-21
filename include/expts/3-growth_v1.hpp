/*
 * File: 3-growth.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-11
 * Description: 
 * NOTE: keep it working & simple
 */

#include "experiments.hpp"
#include <cstddef>
#include <string>

namespace growth_expt {
  enum class GROWTH_EXPT : std::uint8_t {RESERVED, UNRESERVED, INTERNAL_COUNT};
  enum class ContainerType: std::uint8_t {VECTOR, INTERNAL_COUNT};
}

// 
inline void run_vector_growth_experiment(
    std::string relative_file_path,
    std::size_t WARMUP_RUNS = 3,
    std::size_t MEASURED_RUNS = 20) {
    experiments::Experiment_Descriptor<growth_expt::GROWTH_EXPT> expt{
        "dynamic-sizing",
        {"IS_UNRESERVED", "IS_CONTAINER", "SIZE"}
    };

    // user-defined helper lambda
    auto measure_growth = [&](bool use_reserve, int sz) {
        auto expt_type = use_reserve ?
            growth_expt::GROWTH_EXPT::RESERVED :
            growth_expt::GROWTH_EXPT::UNRESERVED;
        
        event_manager::Event::COMMON_PARAMS_DATA_TYPE params[] = {
            static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(
                growth_expt::ContainerType::VECTOR
            ),
            static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(sz)
        };
        
        // Warmup
        for (std::size_t i{}; i < WARMUP_RUNS; ++i) {
            std::vector<int> v;
            if (use_reserve) v.reserve(sz);
            for (std::size_t j{}; j < sz; ++j) v.push_back(j);
            system_utils::do_not_optimize(v.data());
        }

        // Measurement
        for (std::size_t i{}; i < MEASURED_RUNS; ++i) {
            std::vector<int> v;
            if (use_reserve) v.reserve(sz);
            expt.measure(expt_type, params, [&]{
                for (std::size_t j{}; j < sz; ++j) v.push_back(j);
            });
            system_utils::do_not_optimize(v.data());
        }
    };

    for (int sz = 1; sz < 1e8; sz *= 10) {
        measure_growth(true, sz);  // RESERVED
        measure_growth(false, sz); // UNRESERVED
    }

    expt.flush_to_csv(relative_file_path);
}
