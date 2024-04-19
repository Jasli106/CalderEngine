//
//  ComponentDB.hpp
//  game_engine
//
//  Created by Jasmine Li on 3/15/24.
//

#ifndef ComponentDB_hpp
#define ComponentDB_hpp

#include <stdio.h>
#include <filesystem>
#include "rapidjson/document.h"
#include "lua.hpp"
#include "LuaBridge.h"

// From lua hints doc
class Component {
public:
    std::string key;
    std::string type;
    std::shared_ptr<luabridge::LuaRef> componentRef;
    
    bool enabled;
    
    explicit Component() {}
    
    explicit Component(std::string key, std::string type, std::shared_ptr<luabridge::LuaRef> componentRef) : key(key), type(type), componentRef(componentRef) {
        enabled = (*componentRef)["enabled"];
    }
};

class ComponentDB {
private:
    static lua_State *lua_state;
    
    static void Print(std::string message);
    static void PrintErr(std::string message);
    
    static void Quit();
    static void Sleep(int milliseconds);
    static int GetFrame();
    static void OpenURL(std::string url);
public:
    static inline std::unordered_map<std::string, std::shared_ptr<luabridge::LuaRef>> component_tables;
    static inline std::unordered_map<std::string, int> component_counters;
    
    static lua_State* GetLuaState();
    static void Initialize();
    static void InitializeState();
    static void InitializeFunctions();
    static void InitializeComponents();
    static void InitializeComponent(std::filesystem::directory_entry entry);
    static void EstablishInheritance(luabridge::LuaRef &instance_table, luabridge::LuaRef &parent_table);
    static std::shared_ptr<luabridge::LuaRef> CreateRigidbody(std::string name);
    static std::shared_ptr<luabridge::LuaRef> CreateAnimation(std::string name);
    static std::shared_ptr<luabridge::LuaRef> CreateComponentInstance(std::string name, std::string type);
    static std::shared_ptr<luabridge::LuaRef> OverrideComponentInstance(std::shared_ptr<luabridge::LuaRef> component_instance, rapidjson::Value& val);
};

#endif /* ComponentDB_hpp */
