/*
 * File: run_manager.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-24
 * Description: 
 * NOTE: keep it working & simple
 */

#ifndef RUN_MANAGER_HPP
#define RUN_MANAGER_HPP

#include <filesystem>
#include <thread>
#include <vector>

class Run_Manager {
public:
  struct Runnable {
    std::string_view name;
    void (*fn)(std::filesystem::path const&);

    Runnable(std::string_view, void (*func)(std::filesystem::path const&));

    Runnable() = default;
  };
private:
  std::vector<int> chosen_cores;
  std::vector<std::jthread> workers;

  void run_experiments(int, std::vector<Runnable> const&);

public:
  Run_Manager(int, char*[]);
  void start(std::vector<Runnable> const&);

  ~Run_Manager() = default;

  // no copy
  Run_Manager(Run_Manager const&) = delete;
  Run_Manager& operator=(Run_Manager const&) = delete;

  // no move
  Run_Manager(Run_Manager&&) = delete;
  Run_Manager& operator=(Run_Manager&&) = delete;
};

#endif
