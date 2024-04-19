//
//  Event.cpp
//  game_engine
//
//  Created by Jasmine Li on 4/1/24.
//

#include "Event.hpp"

void EventManager::Publish(std::string event_type, luabridge::LuaRef event_object) {
    auto iter = subscribers.find(event_type);
    if (iter != subscribers.end()) {
        for (auto& [compRef, funcRef] : iter->second) {
            if (!compRef.isNil() && !funcRef.isNil()) {
                funcRef(compRef, event_object);
            }
        }
    }
}

void EventManager::Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pendingSubscriptions.push_back(std::make_pair(event_type, std::make_pair(component, function)));
}

void EventManager::Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function) {
    pendingUnsubscriptions.push_back(std::make_pair(event_type, std::make_pair(component, function)));
}

void EventManager::ProcessSubscriptions() {
    for (auto& [event_type, sub] : pendingSubscriptions) {
        subscribers[event_type].push_back(sub);
    }
    pendingSubscriptions.clear();
}

void EventManager::ProcessUnsubscriptions() {
    for (auto& subscription : pendingUnsubscriptions) {
        const auto& event_type = subscription.first;
        const auto& unsub = subscription.second;
        auto& subs = subscribers[event_type];

        auto it = std::remove_if(subs.begin(), subs.end(), [&unsub](const std::pair<luabridge::LuaRef, luabridge::LuaRef>& current) {
            return current == unsub;
        });
        
        subs.erase(it, subs.end());
    }
    pendingUnsubscriptions.clear();
}
