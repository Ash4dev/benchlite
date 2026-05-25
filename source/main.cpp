/*
 * File: main.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-19
 * Description: 
 * NOTE: keep it working & simple
 */

#include "utils/run_manager.hpp"

#include "expts/growth.hpp"

// how to stop early in case of noob/malicious user? sleep 10 minutes?
  // if user can provide a limit : optionally (within a global limit)
  // if timer expires before, exception thrown for the experiment & move to next
// defence mechanism against it:
  // can stop_token be useful for this?
  // conditional variable, mutex,..
// will think later: TODO

int main(int argc, char *argv[]) {
  try {
    Run_Manager manager(argc, argv);
    manager.start({
        Run_Manager::Runnable{"growth.csv", run_vector_growth_experiment}
    });
  }
  catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
