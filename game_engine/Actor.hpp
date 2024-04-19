//
//  Actor.hpp
//  game_engine
//
//  Created by Jasmine Li on 3/16/24.
//

#ifndef Actor_hpp
#define Actor_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <unordered_set>
#include "ComponentDB.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "glm/glm.hpp"

struct Collision;

class Actor
{
private:
    static inline std::vector<Actor*> to_instantiate;
    static inline std::unordered_set<Actor*> to_destroy;
    
    void ReportError(std::string actor_name, const luabridge::LuaException& e);
    std::vector<std::shared_ptr<luabridge::LuaRef>> addcomponent_queue;
    std::unordered_set<std::string> component_graveyard;
    
    void IndividualFrameEnd();
    
public:
    int actor_id;
    std::string actor_name;
    std::string actor_template;
    bool donotdestroy = false;
    
    std::map<std::string, Component> components;
    std::vector<std::string> onstart_queue;
    std::vector<std::string> update_queue;
    std::vector<std::string> lateupdate_queue;
    std::vector<std::string> ondestroy_queue;
    
    Actor(int actor_id, std::string actor_name, std::string actor_template)
    : actor_id(actor_id), actor_name(actor_name), actor_template(actor_template) {
    }
    
    Actor(int actor_id, Actor a)
    : actor_id(actor_id), actor_name(a.actor_name), actor_template(a.actor_template), components(a.components), onstart_queue(a.onstart_queue), update_queue(a.update_queue), lateupdate_queue(a.lateupdate_queue), ondestroy_queue(a.ondestroy_queue) {}
    
    Actor() {}
    
    void InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref);
    static void FrameEnd();
    
    static luabridge::LuaRef Find(std::string name);
    
    static luabridge::LuaRef FindAll(std::string name);
    
    static Actor* Instantiate(std::string template_name);
    
    static void Destroy(Actor* actor);
    
    std::string GetName();
    
    int GetID();
    
    luabridge::LuaRef GetComponentByKey(std::string key);
    
    luabridge::LuaRef GetComponent(std::string type);
    
    luabridge::LuaRef GetComponents(std::string type);
    
    luabridge::LuaRef AddComponent(std::string type);
    
    void RemoveComponent(luabridge::LuaRef component_ref);
    
    void OnStart();
    
    void Update();
    
    void LateUpdate();
    
    void OnDestroy();
    
    void OnCollisionEnter(Collision collision);
    void OnCollisionExit(Collision collision);
    
    void OnTriggerEnter(Collision collision);
    void OnTriggerExit(Collision collision);
};

#endif /* Actor_hpp */
