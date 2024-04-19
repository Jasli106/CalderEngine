//
//  ComponentDB.cpp
//  game_engine
//
//  Created by Jasmine Li on 3/15/24.
//

#include <thread>
#include "ComponentDB.hpp"
#include "SceneDB.hpp"
#include "Input.hpp"
#include "TextDB.hpp"
#include "AudioDB.hpp"
#include "ImageDB.hpp"
#include "Helper.h"
#include "Rigidbody.hpp"
#include "Event.hpp"
#include "Animation.hpp"

lua_State* ComponentDB::lua_state = nullptr;

lua_State* ComponentDB::GetLuaState() {
    return ComponentDB::lua_state;
}

// From discussion 6
void ComponentDB::Initialize() {
    InitializeState();
    InitializeFunctions();
    InitializeComponents();
}

// From discussion 6
void ComponentDB::InitializeState() {
    lua_state = luaL_newstate();
    luaL_openlibs(lua_state);
}

void ComponentDB::InitializeFunctions() {
    // vec2
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<glm::vec2>("vec2")
        .addProperty("x", &glm::vec2::x)
        .addProperty("y", &glm::vec2::y)
        .endClass();
    // Vector2
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<b2Vec2>("Vector2")
        .addConstructor<void(*)(float, float)>()
        .addProperty("x", &b2Vec2::x)
        .addProperty("y", &b2Vec2::y)
        .addFunction("Normalize", &b2Vec2::Normalize)
        .addFunction("Length", &b2Vec2::Length)
        .addFunction("__add", &b2Vec2::operator_add)
        .addFunction("__sub", &b2Vec2::operator_sub)
        .addFunction("__mul", &b2Vec2::operator_mul)
        .endClass()
        .beginNamespace("Vector2")
        .addFunction("Distance", b2Distance)
        .addFunction("Dot", static_cast<float (*)(const b2Vec2&, const b2Vec2&)>(&b2Dot))
        .endNamespace();
    // Debug
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Debug")
        .addFunction("Log", ComponentDB::Print)
        .addFunction("LogError", ComponentDB::PrintErr)
        .endNamespace();
    // Application
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Application")
        .addFunction("Quit", ComponentDB::Quit)
        .addFunction("Sleep", ComponentDB::Sleep)
        .addFunction("GetFrame", ComponentDB::GetFrame)
        .addFunction("OpenURL", ComponentDB::OpenURL)
        .endNamespace();
    // Input
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Input")
        .addFunction("GetKey", Input::GetKey)
        .addFunction("GetKeyDown", Input::GetKeyDown)
        .addFunction("GetKeyUp", Input::GetKeyUp)
        .addFunction("GetMousePosition", Input::GetMousePosition)
        .addFunction("GetMouseButton", Input::GetMouseButton)
        .addFunction("GetMouseButtonDown", Input::GetMouseButtonDown)
        .addFunction("GetMouseButtonUp", Input::GetMouseButtonUp)
        .addFunction("GetMouseScrollDelta", Input::GetMouseScrollDelta)
        .endNamespace();
    // Actor
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Actor>("Actor")
        .addFunction("GetName", &Actor::GetName)
        .addFunction("GetID", &Actor::GetID)
        .addFunction("GetComponentByKey", &Actor::GetComponentByKey)
        .addFunction("GetComponent", &Actor::GetComponent)
        .addFunction("GetComponents", &Actor::GetComponents)
        .addFunction("AddComponent", &Actor::AddComponent)
        .addFunction("RemoveComponent", &Actor::RemoveComponent)
        .endClass()
        .beginNamespace("Actor")
        .addFunction("Find", &Actor::Find)
        .addFunction("FindAll", &Actor::FindAll)
        .addFunction("Instantiate", &Actor::Instantiate)
        .addFunction("Destroy", &Actor::Destroy)
        .endNamespace();
    // Text
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Text")
        .addFunction("Draw", Text::Draw)
        .endNamespace();
    // Audio
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Audio")
        .addFunction("Play", Audio::Play)
        .addFunction("Halt", Audio::Halt)
        .addFunction("SetVolume", Audio::SetVolume)
        .endNamespace();
    // Image
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Image")
        .addFunction("DrawUI", Image::DrawUI)
        .addFunction("DrawUIEx", Image::DrawUIEx)
        .addFunction("Draw", Image::Draw)
        .addFunction("DrawEx", Image::DrawEx)
        .addFunction("DrawPixel", Image::DrawPixel)
        .endNamespace();
    // Camera
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Camera")
        .addFunction("SetPosition", Camera::SetPosition)
        .addFunction("GetPositionX", Camera::GetPositionX)
        .addFunction("GetPositionY", Camera::GetPositionY)
        .addFunction("SetZoom", Camera::SetZoom)
        .addFunction("GetZoom", Camera::GetZoom)
        .endNamespace();
    // Scene
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Scene")
        .addFunction("Load", Scene::Load)
        .addFunction("GetCurrent", Scene::GetCurrent)
        .addFunction("DontDestroy", Scene::DontDestroy)
        .endNamespace();
    // Rigidbody
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Rigidbody>("Rigidbody")
        .addData("enabled", &Rigidbody::enabled)
        .addData("key", &Rigidbody::key)
        .addData("type", &Rigidbody::type)
        .addData("actor", &Rigidbody::actor)
        .addData("x", &Rigidbody::x)
        .addData("y", &Rigidbody::y)
        .addData("body_type", &Rigidbody::body_type)
        .addData("precise", &Rigidbody::precise)
        .addData("body_type", &Rigidbody::body_type)
        .addData("gravity_scale", &Rigidbody::gravity_scale)
        .addData("density", &Rigidbody::density)
        .addData("angular_friction", &Rigidbody::angular_friction)
        .addData("rotation", &Rigidbody::rotation)
        .addData("has_collider", &Rigidbody::has_collider)
        .addData("collider_type", &Rigidbody::collider_type)
        .addData("width", &Rigidbody::width)
        .addData("height", &Rigidbody::height)
        .addData("radius", &Rigidbody::radius)
        .addData("friction", &Rigidbody::friction)
        .addData("bounciness", &Rigidbody::bounciness)
        .addData("has_trigger", &Rigidbody::has_trigger)
        .addData("trigger_type", &Rigidbody::trigger_type)
        .addData("trigger_width", &Rigidbody::trigger_width)
        .addData("trigger_height", &Rigidbody::trigger_height)
        .addData("trigger_radius", &Rigidbody::trigger_radius)
        .addFunction("OnStart", &Rigidbody::Ready)
        .addFunction("OnDestroy", &Rigidbody::OnDestroy)
        .addFunction("GetPosition", &Rigidbody::GetPosition)
        .addFunction("GetRotation", &Rigidbody::GetRotation)
        .addFunction("AddForce", &Rigidbody::AddForce)
        .addFunction("SetVelocity", &Rigidbody::SetVelocity)
        .addFunction("SetPosition", &Rigidbody::SetPosition)
        .addFunction("SetRotation", &Rigidbody::SetRotation)
        .addFunction("SetAngularVelocity", &Rigidbody::SetAngularVelocity)
        .addFunction("SetGravityScale", &Rigidbody::SetGravityScale)
        .addFunction("SetUpDirection", &Rigidbody::SetUpDirection)
        .addFunction("SetRightDirection", &Rigidbody::SetRightDirection)
        .addFunction("GetVelocity", &Rigidbody::GetVelocity)
        .addFunction("GetAngularVelocity", &Rigidbody::GetAngularVelocity)
        .addFunction("GetGravityScale", &Rigidbody::GetGravityScale)
        .addFunction("GetUpDirection", &Rigidbody::GetUpDirection)
        .addFunction("GetRightDirection", &Rigidbody::GetRightDirection)
        .endClass();
    // Collision
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<Collision>("Collision")
        .addData("other", &Collision::other)
        .addData("point", &Collision::point)
        .addData("relative_velocity", &Collision::relative_velocity)
        .addData("normal", &Collision::normal)
        .endClass();
    // HitResult
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<HitResult>("HitResult")
        .addData("actor", &HitResult::actor)
        .addData("point", &HitResult::point)
        .addData("normal", &HitResult::normal)
        .addData("is_trigger", &HitResult::is_trigger)
        .endClass();
    // Physics
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Physics")
        .addFunction("Raycast", Physics::Raycast)
        .addFunction("RaycastAll", Physics::RaycastAll)
        .endNamespace();
    // Event
    luabridge::getGlobalNamespace(lua_state)
        .beginNamespace("Event")
        .addFunction("Publish", EventManager::Publish)
        .addFunction("Subscribe", EventManager::Subscribe)
        .addFunction("Unsubscribe", EventManager::Unsubscribe)
        .endNamespace();
    // Animation
    luabridge::getGlobalNamespace(lua_state)
        .beginClass<AnimationComponent>("Animation")
        .addData("enabled", &AnimationComponent::enabled)
        .addData("key", &AnimationComponent::key)
        .addData("type", &AnimationComponent::type)
        .addData("actor", &AnimationComponent::actor)
        .addData("file", &AnimationComponent::file)
        .addFunction("OnStart", &AnimationComponent::Ready)
        .addFunction("OnUpdate", &AnimationComponent::OnUpdate)
        .addFunction("OnDestroy", &AnimationComponent::OnDestroy)
        .addFunction("Play", &AnimationComponent::Play)
        .addFunction("Stop", &AnimationComponent::Stop)
        .addFunction("GetPosition", &AnimationComponent::GetPosition)
        .addFunction("GetScale", &AnimationComponent::GetScale)
        .addFunction("GetRotation", &AnimationComponent::GetRotation)
        .addFunction("SetPosition", &AnimationComponent::SetPosition)
        .addFunction("SetScale", &AnimationComponent::SetScale)
        .addFunction("SetRotation", &AnimationComponent::SetRotation)
        .endClass();
}

void ComponentDB::InitializeComponents() {
    // Check for component types directory
    std::filesystem::path component_types = std::filesystem::current_path() / "resources/component_types/";
    if (std::filesystem::exists(component_types)) {
        // Add all components in directory to table
        for(const auto& entry : std::filesystem::directory_iterator(component_types)) {
            if(entry.is_regular_file() && entry.path().extension() == ".lua") {
                InitializeComponent(entry);
            }
        }
    }
}

// From discussion 6
void ComponentDB::InitializeComponent(std::filesystem::directory_entry entry) {
    std::string component_name = entry.path().stem().string();
    if(luaL_dofile(lua_state, ("resources/component_types/" + entry.path().stem().string() + ".lua").c_str()) != LUA_OK) {
        std::cout << "problem with lua file " << component_name;
        exit(0);
    }
    component_tables.insert({component_name,
        std::make_shared<luabridge::LuaRef>(
                        luabridge::getGlobal(lua_state, component_name.c_str()))
    });
    component_counters.insert({component_name, 0});
}

// From lecture 13
void ComponentDB::EstablishInheritance(luabridge::LuaRef &instance_table, luabridge::LuaRef &parent_table) {
    luabridge::LuaRef new_metatable = luabridge::newTable(lua_state);
    new_metatable["__index"] = parent_table;
    
    instance_table.push(lua_state);
    new_metatable.push(lua_state);
    lua_setmetatable(lua_state, -2);
    lua_pop(lua_state, 1);
}

std::shared_ptr<luabridge::LuaRef> ComponentDB::CreateRigidbody(std::string name) {
    // Create rigidbody instance
    Rigidbody* rigidbody = new Rigidbody();
    rigidbody->key = name;
    luabridge::LuaRef componentRef(ComponentDB::lua_state, rigidbody);
    std::shared_ptr<luabridge::LuaRef> component_instance = std::make_shared<luabridge::LuaRef>(componentRef);
    (*component_instance)["key"] = name;
    (*component_instance)["type"] = std::string("Rigidbody");
    (*component_instance)["enabled"] = true;
    
    return component_instance;
}

std::shared_ptr<luabridge::LuaRef> ComponentDB::CreateAnimation(std::string name) {
    // Create animation instance
    AnimationComponent* anim = new AnimationComponent();
    anim->key = name;
    luabridge::LuaRef componentRef(ComponentDB::lua_state, anim);
    std::shared_ptr<luabridge::LuaRef> component_instance = std::make_shared<luabridge::LuaRef>(componentRef);
    (*component_instance)["key"] = name;
    (*component_instance)["type"] = std::string("Animation");
    (*component_instance)["enabled"] = true;
    
    return component_instance;
}

std::shared_ptr<luabridge::LuaRef> ComponentDB::CreateComponentInstance(std::string name, std::string type) {
    // Create instance of component
    std::shared_ptr<luabridge::LuaRef> component_instance = std::make_shared<luabridge::LuaRef>(luabridge::newTable(ComponentDB::lua_state));
    ComponentDB::EstablishInheritance(*component_instance, *ComponentDB::component_tables[type]);
    (*component_instance)["key"] = name;
    (*component_instance)["type"] = type;
    (*component_instance)["enabled"] = true;
    return component_instance;
}

std::shared_ptr<luabridge::LuaRef> ComponentDB::OverrideComponentInstance(std::shared_ptr<luabridge::LuaRef> component_instance, rapidjson::Value& val) {
    // Override values
    for (auto& pair : val.GetObject()) {
        switch (pair.value.GetType()) {
            case rapidjson::kNumberType:
                if (pair.value.IsDouble()) {
                    (*component_instance)[pair.name.GetString()] = pair.value.GetDouble();
                } else if (pair.value.IsInt()) {
                    (*component_instance)[pair.name.GetString()] = pair.value.GetInt();
                }
                break;
            case rapidjson::kStringType:
                (*component_instance)[pair.name.GetString()] = pair.value.GetString();
                break;
            case rapidjson::kTrueType:
            case rapidjson::kFalseType:
                (*component_instance)[pair.name.GetString()] = pair.value.GetBool();
                break;
            default:
                break;
        }
    }
    return component_instance;
}

void ComponentDB::Print(std::string message) {
    std::cout << message << std::endl;
}
void ComponentDB::PrintErr(std::string message) {
    std::cerr << message << std::endl;
}

void ComponentDB::Quit() {
    exit(0);
}

void ComponentDB::Sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int ComponentDB::GetFrame() {
    return Helper::GetFrameNumber();
}

void ComponentDB::OpenURL(std::string url) {
#if defined(_WIN32)
    std::system(("start " + url).c_str());
#elif defined(__APPLE__)
    std::system(("open " + url).c_str());
#elif defined(__linux__)
    std::system(("xdg-open " + url).c_str());
#endif
}
