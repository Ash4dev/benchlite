/*
 * File: experiments.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-12
 * Description: 
 * NOTE: keep it working & simple
 */

#ifndef EXPERIMENTS_HPP
#define EXPERIMENTS_HPP

#include "utils/event_manager.hpp"
#include "utils/general.hpp"
#include "utils/timer.hpp"

#include <exception>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <initializer_list>
#include <span>
#include <string_view>

#include <cstddef>

// std::function : heap-alloc based, indirect, no-inline -> useful for lambdas
// vanilla fn pointer: zero-alloc, direct, trivial BUT difficult to extend

namespace experiments {
  template <event_manager::Main_BM_Cat Scenarios>
  class Experiment_Descriptor {
  private:
    std::string_view name;
    std::size_t param_cnt;

    // TODO:

    // these are >= 0 run time knobs to be altered
      // numerical: {{start, end, jump}}
      // categorical: enum class
    // must be convertible to COMMON_PARAMS_DATA_TYPE
    using LabelArray = std::array<std::string_view, event_manager::Event::MAX_PARAM_COUNT>;
    LabelArray param_labels;

    event_manager::Tracker<Scenarios> tracker;

    template<typename BMable_Fn>
    void measure(Scenarios scenario,
        std::span<event_manager::Event::COMMON_PARAMS_DATA_TYPE> params,
        BMable_Fn &&fn);

  public:
    // initializer_list: allows brace-init / const viewer by default / shallow copy of temp / light wrapper
    Experiment_Descriptor(
        std::string_view name,
        std::initializer_list<std::string_view> labels // param_cnt from here, less than MAX_PARAM_COUNT though
    );

    Experiment_Descriptor() = delete;
    ~Experiment_Descriptor() = default;

    // copy deleted
    Experiment_Descriptor(Experiment_Descriptor const &) = delete;
    Experiment_Descriptor &operator=(Experiment_Descriptor const &) = delete;

    // move defaulted
    Experiment_Descriptor(Experiment_Descriptor &&) = default;
    Experiment_Descriptor &operator=(Experiment_Descriptor &&) = default;

    // std::span: non-owning (default: non-const) viewer over contiguous sequence of objects
    // NOTE: hot-path

    template<typename BMable_Fn>
    void measure_after_warmup(Scenarios scenario,
        std::span<event_manager::Event::COMMON_PARAMS_DATA_TYPE> params,
        BMable_Fn &&fn,
        std::size_t warmup_runs = 3,
        std::size_t measured_runs = 20);

    void flush_to_csv(std::filesystem::path const&file_name);
  };
}

namespace experiments {
  template<event_manager::Main_BM_Cat Scenarios>
  Experiment_Descriptor<Scenarios>::Experiment_Descriptor(
      std::string_view _name,
      std::initializer_list<std::string_view> _labels
  ): name{_name}, param_cnt {_labels.size()}, tracker {event_manager::Tracker<Scenarios>()} {
    // eval order: as defined -> name -> param_cnt -> param_labels -> tracker
    // not a problem since tracker and param_labels NOT cross-dependent
    std::size_t i {};
    for (auto itr = _labels.begin(); itr != _labels.end(); ++itr, ++i){
      param_labels[i] = *itr;
    }
  }

  // this is complicated prefer definitions as-is
  template<event_manager::Main_BM_Cat Scenarios>
  template<typename BMable_Fn>
  void Experiment_Descriptor<Scenarios>::measure(
      Scenarios scenario,
      std::span<event_manager::Event::COMMON_PARAMS_DATA_TYPE> params,
      BMable_Fn &&fn
  ) {

    // config params are ONLY used for event creation & tracking
    event_manager::Event e;

    // NOTE: assign values to the event
    e.params[0] = static_cast<std::size_t>(scenario);
    std::size_t i {1};
    for (auto itr {params.begin()}; itr != params.end(); ++itr, ++i){
      // params will change -> cannot be const. drop the const from span?
      e.params[i] = *itr;
    }

    // NOTE: timing scope
    {
      ScopedTimer t{e.time_elapsed};
      fn();
    }

    tracker.add(e);
  }

  template<event_manager::Main_BM_Cat Scenarios>
  template<typename BMable_Fn>
  void Experiment_Descriptor<Scenarios>::measure_after_warmup(Scenarios scenario,
      std::span<event_manager::Event::COMMON_PARAMS_DATA_TYPE> params,
      BMable_Fn &&fn,
      std::size_t warmup_runs,
      std::size_t measured_runs) {

    // TODO: how to ensure no compiler optimizations?
    for (std::size_t i{}; i < warmup_runs; ++i){
      fn();
    }

    for (std::size_t i{}; i < measured_runs; ++i){
      measure(scenario, params, fn);
    }
  }

  template<event_manager::Main_BM_Cat Scenarios>
  void Experiment_Descriptor<Scenarios>::flush_to_csv(std::filesystem::path const &file_name){

    std::filesystem::path p{file_name};
    std::filesystem::path dir {"run/temp"};

    if ((not std::filesystem::exists(dir)) or (not dir.empty())){
      std::filesystem::create_directories(dir);
    }

    std::ofstream csv_file(dir / file_name, std::ios::out); // directly opens
    if (not csv_file.is_open()) {
      std::cerr << "Error: Could not open file " << file_name << " for writing.\n";
      return;
    }

    try {
      for (std::size_t i{}; i < param_cnt; ++i){
        csv_file << param_labels[i] << ",";
      }
      csv_file << "ELAPSED_TIME_NS\n";

      for (auto const &op_track: tracker) {
        for (auto const &event: op_track) {
          for (std::size_t i {0}; i < param_cnt; ++i){
              // TODO: how to get it to str for categorical variables
              // if categorical must also define string typecast? readability
              // try for str conversion, if failed move with direct value in catch
              csv_file << event.params[i] << ",";
          }
          csv_file << event.time_elapsed.count() << "\n";
        }
      }
    }
    catch (const std::exception &e) {
      std::cerr << "Exception occurred while writing to CSV: "
        << e.what() << "\n";
      csv_file.close();
    }

    csv_file.close();
  }
}

#endif // !EXPERIMENTS_HPP
