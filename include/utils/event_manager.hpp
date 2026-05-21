/*
 * File: event.hpp
 * Author: Ashutosh Panigrahy
 * Created: 2026-05-07
 * Description: 
 * NOTE: keep it working & simple
 */

#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <array>
#include <vector>
#include <cstddef>
#include <chrono>

#include <type_traits>

namespace event_manager {

  struct Event {
    using COMMON_PARAMS_DATA_TYPE = float;

    // inline: allows definition within declaration
      // ODR resolver: de-duplicate multi definition on includes
    // static: one copy of variable in memory
    // const: prevents modification
    std::size_t inline static const MAX_PARAM_COUNT {10};

    std::chrono::nanoseconds time_elapsed;
    std::array<COMMON_PARAMS_DATA_TYPE, MAX_PARAM_COUNT> params;
  };
}

namespace event_manager {
  class StoreHouse {
  private:
    std::size_t static const BATCH_SIZE = 1'000'000;
    std::vector<Event> event_store;
  public:
    StoreHouse();
    ~StoreHouse() = default;
    
    StoreHouse(StoreHouse const&) = delete;
    StoreHouse &operator=(StoreHouse const&) = delete;

    StoreHouse(StoreHouse &&) = default;
    StoreHouse &operator=(StoreHouse &&) = default;

    std::vector<Event>::const_iterator begin() const;
    std::vector<Event>::const_iterator end() const;
    std::size_t size() const;

    void add_event(Event const&);
    Event const& read_event();

    void clear();
  };
}

namespace event_manager {
  // NOTE: Primary benchmark category
  template<typename E>
  concept Main_BM_Cat = std::is_enum_v<E> && requires () {
    E::INTERNAL_COUNT; // check if expression exists

    // requires is a must to check value, NOT just existence check
    requires static_cast<std::size_t>(E::INTERNAL_COUNT) > 0;
  };

  // prevents accidental mixing of events at compile-time: templates
  template<Main_BM_Cat EventType>
  class Tracker {
  private:
    std::vector<StoreHouse> operation_trackers; // NOTE: 1 Store_House per operation

  public:
    Tracker();
    ~Tracker() = default;

    // copy deleted
    Tracker(Tracker const&) = delete;
    Tracker &operator=(Tracker const&) = delete;

    // move defaulted
    Tracker(Tracker &&) = default;
    Tracker &operator=(Tracker &&) = default;

    // const member fns prevent change to non-static members within fn
    // a reference to non-const can still affect the object externally
    // problems with return by const reference:
    // bad index, early termination return value

    // range-based for loop easier-simple solution
    auto begin() const;
    auto end() const;

    void add(Event const&);
    std::size_t size() const;
  };
}

// templates instantiated by compiler: cannot see into another TU
// definition and type must be visible to compiler at point of use
namespace event_manager {
  template<Main_BM_Cat EventType>
  Tracker<EventType>::Tracker() {
    operation_trackers.resize(
      static_cast<std::size_t>(EventType::INTERNAL_COUNT)
    );
  }

  template <Main_BM_Cat EventType>
  void Tracker<EventType>::add(const Event& e) {
    operation_trackers[
      static_cast<std::size_t>(static_cast<EventType>(e.params[0]))
    ].add_event(e);
  }

  template <Main_BM_Cat EventType>
  std::size_t Tracker<EventType>::size() const{
    return operation_trackers.size();
  }

  template<Main_BM_Cat EventType>
  auto Tracker<EventType>::begin() const {
    return operation_trackers.begin();
  }

  template<Main_BM_Cat EventType>
  auto Tracker<EventType>::end() const {
    return operation_trackers.end();
  }
}
#endif
