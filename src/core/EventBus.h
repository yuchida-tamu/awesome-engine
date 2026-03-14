#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
using SubscriptionID = std::uint32_t;

class EventBus {
public:
  template <typename EventT>
  SubscriptionID Subscribe(std::function<void(const EventT &)> handler) {
    SubscriptionID id = m_nextId++;
    auto &subs = m_subscribers[std::type_index(typeid(EventT))];
    subs.push_back({id, [handler](const void *event) {
                      handler(*static_cast<const EventT *>(event));
                    }});
    return id;
  }
  void Unsubscribe(SubscriptionID id) {
    for (auto &[typeIdx, subs] : m_subscribers) {
      subs.erase(
          std::remove_if(subs.begin(), subs.end(),
                         [id](const Subscription &s) { return s.id == id; }),
          subs.end());
    }
  }
  template <typename EventT> void Publish(const EventT &event) {
    auto it = m_subscribers.find(std::type_index(typeid(EventT)));
    if (it != m_subscribers.end()) {
      auto subs = it->second; // copy to avoid iterator invalidation
      for (auto &sub : subs) {
        sub.callback(&event);
      }
    }
  }

private:
  struct Subscription {
    SubscriptionID id;
    std::function<void(const void *)> callback;
  };

  std::unordered_map<std::type_index, std::vector<Subscription>> m_subscribers;
  SubscriptionID m_nextId = 1;
};
