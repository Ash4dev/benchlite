/*
 * File: run_manager.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-24
 * Description: 
 * NOTE: keep it working & simple
 */
#include "utils/run_manager.hpp"
#include "utils/general.hpp"

#include <iostream>

Run_Manager::Runnable::Runnable(std::string_view file, void (*func)(std::filesystem::path const&)) : 
  name(file), fn(func) {}

Run_Manager::Run_Manager(int argc, char* argv[]) {
  if (argc < 2) {
    throw std::runtime_error("Usage: " + std::string(argv[0]) + " <chosen cores array>");
  }

  chosen_cores.reserve(argc - 1);
  for (int i = 1; i < argc; ++i)
    chosen_cores.push_back(std::stoi(argv[i]));
}

void Run_Manager::run_experiments(int logical_core_id, std::vector<Runnable> const& expts) {
  system_utils::pin_thread_to_core(logical_core_id);
  system_utils::raise_thread_priority();

  for (auto const& expt : expts) {
    try {
      expt.fn(expt.name);
    }
    catch (std::exception const& e) {
      std::cerr << expt.name << " running on core " << logical_core_id
      << " generated error: " << e.what() << '\n';
    }
  }
}

void Run_Manager::start(std::vector<Runnable> const& expts) {
  std::size_t ncores = chosen_cores.size();
  std::size_t nexpts = expts.size();

  workers.reserve(std::min(ncores, nexpts));

  std::size_t start_idx {};
  std::size_t jump {(nexpts + ncores - 1) / ncores};

  for (int const core : chosen_cores) {
    if (start_idx >= nexpts)
        break;
    std::size_t end_idx = std::min(start_idx + jump, nexpts);

    // copy made : assuming really small vector
    std::vector<Runnable> assigned(
        expts.begin() + start_idx,
        expts.begin() + end_idx
    );

    workers.emplace_back(
    [this, core, assigned]() {
        run_experiments(core, assigned);
      }
    );
    // push_back: temp of existing obj + copy/move (no implicit conversion)
    // emplace_back: inplace object creation (unrelated sign match)
      // vector<string>: v.push_back(10); COMPILE ERROR (no implicit conversion)
      // v.emplace_back(10); WORSE -> Calls std::string(size_t n, char c)

    start_idx += jump;
  }
}
