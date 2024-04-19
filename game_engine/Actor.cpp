//
//  Actor.cpp
//  game_engine
//
//  Created by Jasmine Li on 3/16/24.
//

#include "Actor.hpp"
#include "SceneDB.hpp"
#include "TemplateDB.h"
#include "Rigidbody.hpp"

void Actor::ReportError(std::string actor_name, const luabridge::LuaException& e) {
    std::string err_msg = e.what();
    
    std::replace(err_msg.begin(), err_msg.end(), '\\', '/');
    
    std::cout << "\033[31m" << actor_name << " : " << err_msg << "\033[0m" << std::endl;
}
    
void Actor::InjectConvenienceReferences(std::shared_ptr<luabridge::LuaRef> component_ref)
{
    (*component_ref)["actor"] = this;
}

luabridge::LuaRef Actor::Find(std::string name) {
    for(int i=0; i<actors.size(); ++i) {
        if(actors[i]->actor_name == name && to_destroy.find(actors[i]) == to_destroy.end()) {
            return luabridge::LuaRef(ComponentDB::GetLuaState(), actors[i]);
        }
    }
    for(int i=0; i<to_instantiate.size(); ++i) {
        if(to_instantiate[i]->actor_name == name && to_destroy.find(actors[i]) == to_destroy.end()) {
            return luabridge::LuaRef(ComponentDB::GetLuaState(), to_instantiate[i]);
        }
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}
    
luabridge::LuaRef Actor::FindAll(std::string name) {
    std::shared_ptr<luabridge::LuaRef> results = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::GetLuaState()));
    int n = 1;
    for(int i=0; i<actors.size(); ++i) {
        if(actors[i]->actor_name == name && to_destroy.find(actors[i]) == to_destroy.end()) {
            (*results)[n] = luabridge::LuaRef(ComponentDB::GetLuaState(), actors[i]);
            ++n;
        }
    }
    for(int i=0; i<to_instantiate.size(); ++i) {
        if(to_instantiate[i]->actor_name == name && to_destroy.find(actors[i]) == to_destroy.end()) {
            (*results)[n] = luabridge::LuaRef(ComponentDB::GetLuaState(), to_instantiate[i]);
            ++n;
        }
    }
    return *results;
}

Actor* Actor::Instantiate(std::string template_name) {
    Actor* new_actor = new Actor(LoadActorFromTemplate(template_name));
    for(auto& c : new_actor->components) {
        new_actor->InjectConvenienceReferences(c.second.componentRef);
    }
    to_instantiate.push_back(new_actor);
    return new_actor;
}

void Actor::Destroy(Actor* actor) {
    to_destroy.insert(actor);
    for(auto &c : actor->components) {
        (*c.second.componentRef)["enabled"] = false;
        actor->ondestroy_queue.push_back(c.first);
    }
}
    
std::string Actor::GetName() {
    return actor_name;
}
    
int Actor::GetID() {
    return actor_id;
}
    
luabridge::LuaRef Actor::GetComponentByKey(std::string key) {
    if(components.find(key) != components.end() && component_graveyard.find(key) == component_graveyard.end()) {
        return *components[key].componentRef;
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}
    
luabridge::LuaRef Actor::GetComponent(std::string type) {
    for(auto &c : components) {
        if(c.second.type == type && component_graveyard.find(c.first) == component_graveyard.end()) {
            return *c.second.componentRef;
        }
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}
    
luabridge::LuaRef Actor::GetComponents(std::string type) {
    std::shared_ptr<luabridge::LuaRef> results = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::GetLuaState()));
    int n = 1;
    for(auto &c : components) {
        if(c.second.type == type && component_graveyard.find(c.first) == component_graveyard.end()) {
            (*results)[n] = *c.second.componentRef;
            ++n;
        }
    }
    return *results;
}

luabridge::LuaRef Actor::AddComponent(std::string type) {
    std::string key = "r" + std::to_string(ComponentDB::component_counters[type]);
    ++ComponentDB::component_counters[type];
    std::shared_ptr<luabridge::LuaRef> component_instance;
    if(type == "Rigidbody") {
        component_instance = ComponentDB::CreateRigidbody(key);
    }
    else if(type == "Animation") {
        component_instance = ComponentDB::CreateAnimation(key);
    }
    else {
        component_instance = ComponentDB::CreateComponentInstance(key, type);
    }
    addcomponent_queue.push_back(component_instance);
    return *component_instance;
}

void Actor::RemoveComponent(luabridge::LuaRef component_ref) {
    component_ref["enabled"] = false;
    std::string key = component_ref["key"];
    component_graveyard.insert(key);
    ondestroy_queue.push_back(key);
    return;
}
    
void Actor::OnStart() {
    for(const auto & c : onstart_queue) {
        components[c].enabled = (*components[c].componentRef)["enabled"];
        luabridge::LuaRef OnStart_lua = (*components[c].componentRef)["OnStart"];
        if(components[c].enabled && !OnStart_lua.isNil()) {
            try {
                OnStart_lua((*components[c].componentRef));
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
    onstart_queue.clear();
}
    
void Actor::Update() {
    for(const auto & c : update_queue) {
        components[c].enabled = (*components[c].componentRef)["enabled"];
        luabridge::LuaRef OnUpdate_lua = (*components[c].componentRef)["OnUpdate"];
        if(components[c].enabled && !OnUpdate_lua.isNil()) {
            try {
                OnUpdate_lua((*components[c].componentRef));
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
}

void Actor::LateUpdate() {
    for(const auto & c : lateupdate_queue) {
        components[c].enabled = (*components[c].componentRef)["enabled"];
        luabridge::LuaRef OnLateUpdate_lua = (*components[c].componentRef)["OnLateUpdate"];
        if(components[c].enabled && !OnLateUpdate_lua.isNil()) {
            try {
                OnLateUpdate_lua((*components[c].componentRef));
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
    OnDestroy();
    IndividualFrameEnd();
}

void Actor::OnDestroy() {
    for(const auto & c : ondestroy_queue) {
        luabridge::LuaRef OnDestroy_lua = (*components[c].componentRef)["OnDestroy"];
        if(!OnDestroy_lua.isNil()) {
            try {
                OnDestroy_lua((*components[c].componentRef));
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
    ondestroy_queue.clear();
}

void Actor::OnCollisionEnter(Collision collision) {
    for(const auto & c : components) {
        luabridge::LuaRef OnCollisionEnter_lua = (*c.second.componentRef)["OnCollisionEnter"];
        if((c.second.enabled) && !OnCollisionEnter_lua.isNil()) {
            try {
                OnCollisionEnter_lua(*c.second.componentRef, collision);
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
}

void Actor::OnCollisionExit(Collision collision) {
    for(const auto & c : components) {
        luabridge::LuaRef OnCollisionExit_lua = (*c.second.componentRef)["OnCollisionExit"];
        if((c.second.enabled) && !OnCollisionExit_lua.isNil()) {
            try {
                OnCollisionExit_lua(*c.second.componentRef, collision);
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
}

void Actor::OnTriggerEnter(Collision collision) {
    for(const auto & c : components) {
        luabridge::LuaRef OnTriggerEnter_lua = (*c.second.componentRef)["OnTriggerEnter"];
        if((c.second.enabled) && !OnTriggerEnter_lua.isNil()) {
            try {
                OnTriggerEnter_lua(*c.second.componentRef, collision);
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
}

void Actor::OnTriggerExit(Collision collision) {
    for(const auto & c : components) {
        luabridge::LuaRef OnTriggerExit_lua = (*c.second.componentRef)["OnTriggerExit"];
        if((c.second.enabled) && !OnTriggerExit_lua.isNil()) {
            try {
                OnTriggerExit_lua(*c.second.componentRef, collision);
            } catch (const luabridge::LuaException& e){
                ReportError(actor_name, e);
                break;
            }
        }
    }
}

void Actor::IndividualFrameEnd() {
    for(const auto & c : addcomponent_queue) {
        components[(*c)["key"]] = Component((*c)["key"], (*c)["type"], c);
        onstart_queue.push_back((*c)["key"]);
        update_queue.push_back((*c)["key"]);
        lateupdate_queue.push_back((*c)["key"]);
    }
    std::sort(onstart_queue.begin(), onstart_queue.end());
    std::sort(update_queue.begin(), update_queue.end());
    std::sort(lateupdate_queue.begin(), lateupdate_queue.end());
    addcomponent_queue.clear();
    
    for(const auto & c : component_graveyard) {
        components.erase(components.find(c));
        auto it = std::lower_bound(update_queue.begin(), update_queue.end(), c);
        if (it != update_queue.end() && *it == c) {
            update_queue.erase(it);
        }
        it = std::lower_bound(lateupdate_queue.begin(), lateupdate_queue.end(), c);
        if (it != lateupdate_queue.end() && *it == c) {
            lateupdate_queue.erase(it);
        }
    }
    component_graveyard.clear();
}

void Actor::FrameEnd() {
    for(const auto & a : to_instantiate) {
        actors.push_back(a);
        n_actors++;
    }
    to_instantiate.clear();
    
    for(const auto & a : actors) {
        auto it = std::find(actors.begin(), actors.end(), a);

        if (to_destroy.find(a) != to_destroy.end() && it != actors.end()) {
            actors.erase(it);
            delete a;
        }
    }
    to_destroy.clear();
}
