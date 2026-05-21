/*
 * File: main.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-02
 * Description: 
 * NOTE: keep it working & simple
 */

#include "experiments.hpp"

namespace growth_expt {
  enum class GROWTH_EXPT : std::uint8_t {RESERVED, UNRESERVED, INTERNAL_COUNT};
  enum class ContainerType: std::uint8_t {VECTOR, INTERNAL_COUNT};
}

inline void run_vector_growth_experiment() {
    experiments::Experiment_Descriptor<growth_expt::GROWTH_EXPT> expt{
        "dynamic-sizing",
        {
            "IS_UNRESERVED",
            "IS_CONTAINER",
            "SIZE"
        }
    };

    constexpr size_t WARMUP_RUNS   = 3;
    constexpr size_t MEASURED_RUNS = 20;

    for (int sz = 1; sz < 1e8; sz *= 10) {
        event_manager::Event::COMMON_PARAMS_DATA_TYPE params[] = {
            static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(
                growth_expt::ContainerType::VECTOR
            ),
            static_cast<event_manager::Event::COMMON_PARAMS_DATA_TYPE>(sz)
        };

        //
        // RESERVED
        //
        for (size_t i{}; i < WARMUP_RUNS; ++i) {
            std::vector<int> v;
            v.reserve(sz);
            for (size_t j{}; j < sz; ++j) v.push_back(j);
            system_utils::do_not_optimize(v.data());
        }
        for (size_t i{}; i < MEASURED_RUNS; ++i) {
            std::vector<int> v;
            v.reserve(sz);
            expt.measure(
                growth_expt::GROWTH_EXPT::RESERVED,
                params,
                [&]{ for (size_t j{}; j < sz; ++j) v.push_back(j); }
            );
            system_utils::do_not_optimize(v.data());
        }

        //
        // UNRESERVED
        //
        for (size_t i{}; i < WARMUP_RUNS; ++i) {
            std::vector<int> v;
            for (size_t j{}; j < sz; ++j) v.push_back(j);
            system_utils::do_not_optimize(v.data());
        }
        for (size_t i{}; i < MEASURED_RUNS; ++i) {
            std::vector<int> v;
            expt.measure(
                growth_expt::GROWTH_EXPT::UNRESERVED,
                params,
                [&]{ for (size_t j{}; j < sz; ++j) v.push_back(j); }
            );
            system_utils::do_not_optimize(v.data());
        }
    }

    expt.flush_to_csv("run/growth.csv");
}

int main() {
  run_vector_growth_experiment();
  return 0;
}
