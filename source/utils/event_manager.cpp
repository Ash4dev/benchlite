/*
 * File: event_manager.cpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-12
 * Description: 
 * NOTE: keep it working & simple
 */

#include "utils/event_manager.hpp"
#include <vector>
#include <cstddef>

namespace event_manager {
  StoreHouse::StoreHouse() {
    event_store.reserve(StoreHouse::BATCH_SIZE);
  }

  void StoreHouse::add_event(Event const &e){
    // push_back: copies/moves existing object
      // copy ctor triggered since e: named reference
    // emplace_back: constructs object in place
    event_store.push_back(e);
  }

  std::size_t StoreHouse::size() const{
    return event_store.size();
  }

  std::vector<Event>::const_iterator StoreHouse::begin() const {
    return event_store.begin();
  }

  std::vector<Event>::const_iterator StoreHouse::end() const {
    return event_store.end();
  }
}
