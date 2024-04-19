//
//  Event.hpp
//  game_engine
//
//  Created by Jasmine Li on 4/1/24.
//

#ifndef Event_hpp
#define Event_hpp

#include <stdio.h>
#include <string>
#include "lua.hpp"
#include "LuaBridge.h"

class EventManager {
public:
    static void Publish(std::string event_type, luabridge::LuaRef event_object);
    static void Subscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    static void Unsubscribe(std::string event_type, luabridge::LuaRef component, luabridge::LuaRef function);
    
    static void ProcessSubscriptions();
    static void ProcessUnsubscriptions();
private:
    static inline std::unordered_map<std::string, std::vector<std::pair<luabridge::LuaRef, luabridge::LuaRef>>> subscribers;
    static inline std::vector<std::pair<std::string, std::pair<luabridge::LuaRef, luabridge::LuaRef> > > pendingSubscriptions;
    static inline std::vector<std::pair<std::string, std::pair<luabridge::LuaRef, luabridge::LuaRef>>> pendingUnsubscriptions;
};

#endif /* Event_hpp */
