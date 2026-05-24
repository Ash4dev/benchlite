/*
 * File: main.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-19
 * Description: 
 * NOTE: keep it working & simple
 */

#include "expts/growth.hpp"
#include "utils/experiments.hpp"
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <format>
#include <string>
#include <thread>

// why not pass as a vector of experiments: share the same signature void (*fn)(string)
void run_experiments(int core_id) {
  system_utils::pin_thread_to_core(core_id);
  std::cout << std::this_thread::get_id() << ": started on core_id: " << core_id << "\n";
  system_utils::raise_thread_priority();

  // series of experiments: simulated with repeat of a single experiment
  for (std::size_t i {}; i < std::max(2, core_id%5); ++i){
    try {
      // this is the only thing that changes across multiple runs
      // std::format temporary fix -> untill multiple experiments are not written
      // run/temp remains fixed
      run_vector_growth_experiment(std::format("run/temp/f{0}_{1}.csv", core_id, i));
    }
    catch(const std::exception& e) {
      std::cerr << "went bad\n"; // some more identifiers
    }
  }

  // how to stop early in case of noob/malicious user? sleep 10 minutes?
    // if user can provide a limit : optionally (within a global limit)
    // if timer expires before, exception thrown for the experiment & move to next
  // defence mechanism against it:
    // can stop_token be useful for this?
    // conditional variable, mutex,..
  // will think later: TODO
}

// main function is also a decent amount of boilerplate
// the only thing the user needs to ideally do is provide the info for each expt
  // name, fn
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <chosen cores array>\n";
    return EXIT_FAILURE;
  }
  int const core_cnt = argc-1;

  // actually allocates core_cnt elements
  std::vector<int> chosen_cores(core_cnt);
  // reserve: no allocs / resize: does allocs
  // valid arguments ensured by bash script
  for (std::size_t i {1}; i < argc; ++i)
    chosen_cores[i-1] = (std::stoi(argv[i]));

  // automatically joins RAII: no inexplicable std::terminate
  std::vector<std::jthread> jtarr;
  for (std::size_t i {}; i < core_cnt; ++i){
    // push_back would require temp object: temp + copy/move
    // emplace_back: no temp object - constructs object in place
      // uses "perfect-forwarding" (WTF?) / variable arg number

    // push_back [use]: when have object already / no accidental "explicit" conversions
      // vector<string>: v.push_back(10); COMPILE ERROR (no implicit conversion)
      // v.emplace_back(10); WORSE -> Calls std::string(size_t n, char c)
    
    // how will this signature change? std::vector of fns & name
    jtarr.emplace_back(run_experiments, chosen_cores[i]);
  }

  return EXIT_SUCCESS;
}
