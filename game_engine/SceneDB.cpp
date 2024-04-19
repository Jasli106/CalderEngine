//
//  SceneDB.cpp
//  game_engine
//
//  Created by Jasmine Li on 2/3/24.
//

#include <iostream>
#include <vector>
#include "SceneDB.hpp"
#include "TemplateDB.h"
#include "ImageDB.hpp"
#include "ComponentDB.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "Rigidbody.hpp"

// Camera
int w;
int h;

Uint8 clear_color_r = 255;
Uint8 clear_color_g = 255;
Uint8 clear_color_b = 255;

glm::ivec2 camera_offset = glm::ivec2(0,0);
float cam_ease_factor = 1.0;

// Game
std::filesystem::path resources;
rapidjson::Document config;

std::string game_title;

std::string game_start_message;
std::string game_over_bad_message;
std::string game_over_good_message;

//Actors
int n_actors = 0;
std::vector<Actor*> actors;
SDL_Texture* hp_img;

std::vector<IDrawRequest*> render_queue;
std::queue<Pixel> pixel_render_queue;

void LoadInitialScene() {
    
    // Check for resources directory
    resources = std::filesystem::current_path() / "resources";
    if (!std::filesystem::exists(resources)) {
        std::cout << "error: resources/ missing";
        exit(0);
    }
    
    // Check for resources/game.config
    if(!std::filesystem::exists(resources/"game.config")) {
        std::cout << "error: resources/game.config missing";
        exit(0);
    }
    
    // Game config
    EngineUtils::ReadJsonFile((resources/"game.config").generic_string(), config);
    
    if(config.HasMember("initial_scene")) {
        std::string scene_name = config["initial_scene"].GetString();
    } else {
        std::cout << "error: initial_scene unspecified";
        exit(0);
    }
    
    if(config.HasMember("game_start_message")) {
        game_start_message = config["game_start_message"].GetString();
    } else {
        game_start_message = "";
    }
    if(config.HasMember("game_over_bad_message")) {
        game_over_bad_message = config["game_over_bad_message"].GetString();
    } else {
        game_over_bad_message = "";
    }
    if(config.HasMember("game_over_good_message")) {
        game_over_good_message = config["game_over_good_message"].GetString();
    } else {
        game_over_good_message = "";
    }
    
    
    if(config.HasMember("game_title")) {
        game_title = config["game_title"].GetString();
    } else {
        game_title = "";
    }
    
    // Rendering config
    if(std::filesystem::exists(resources/"rendering.config")) {
        rapidjson::Document rendering;
        EngineUtils::ReadJsonFile((resources/"rendering.config").generic_string(), rendering);
        if(rendering.HasMember("x_resolution")) {
            Camera::resolution.x = rendering["x_resolution"].GetInt();
            w = floor(rendering["x_resolution"].GetInt()/2);
        }
        if(rendering.HasMember("y_resolution")) {
            Camera::resolution.y = rendering["y_resolution"].GetInt();
            h = floor(rendering["y_resolution"].GetInt()/2);
        }
        if(rendering.HasMember("clear_color_r")) {
            clear_color_r = rendering["clear_color_r"].GetInt();
        }
        if(rendering.HasMember("clear_color_g")) {
            clear_color_g = rendering["clear_color_g"].GetInt();
        }
        if(rendering.HasMember("clear_color_b")) {
            clear_color_b = rendering["clear_color_b"].GetInt();
        }
        if(rendering.HasMember("cam_offset_x")) {
            camera_offset.x = rendering["cam_offset_x"].GetFloat();
        }
        if(rendering.HasMember("cam_offset_y")) {
            camera_offset.y = rendering["cam_offset_y"].GetFloat();
        }
        if(rendering.HasMember("zoom_factor")) {
            Camera::zoom_factor = rendering["zoom_factor"].GetFloat();
        }
        if(rendering.HasMember("cam_ease_factor")) {
            cam_ease_factor = rendering["cam_ease_factor"].GetFloat();
        }
    }
    else {
        w = 6;
        h = 4;
    }
}

Actor CreateActor(rapidjson::Value& a) {
    Actor new_actor;
    if (a.HasMember("template")) { // Actor has template
        std::string template_name = a["template"].GetString();
        // Load template
        new_actor = LoadActorFromTemplate(template_name);
        // Add components
        if(a.HasMember("components")) {
            for (auto& c : a["components"].GetObject()) {
                // Check if component is inheriting from template
                std::string type;
                if(new_actor.components.find(c.name.GetString()) != new_actor.components.end()) {
                    type = new_actor.components[c.name.GetString()].type;
                } else {
                    type = c.value["type"].GetString();
                }
                if(type == "Rigidbody") {
                    // Create instance of component
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateRigidbody(c.name.GetString());
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);
                    
                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                }
                else if(type == "Animation") {
                    // Create instance of component
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateAnimation(c.name.GetString());
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);
                    
                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                }
                else if(ComponentDB::component_tables.find(type) != ComponentDB::component_tables.end()) { // Component exists
                    // Create instance of component
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateComponentInstance(c.name.GetString(), type);
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);
                    
                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                } else {
                    std::cout << "error: failed to locate component " << c.value["type"].GetString();
                    exit(0);
                }
            }
        }
        // Add components to lifecycle queues
        new_actor.onstart_queue.clear();
        for(auto& c : new_actor.components) {
            new_actor.onstart_queue.push_back(c.first);
            new_actor.update_queue.push_back(c.first);
            new_actor.lateupdate_queue.push_back(c.first);
        }
        new_actor.actor_name = a.HasMember("name") ? a["name"].GetString() : new_actor.actor_name;
    }
    else { // No template
        std::string actor_name = a.HasMember("name") ? a["name"].GetString() : "";
        new_actor = Actor(n_actors, actor_name, "");
        // Add components
        if(a.HasMember("components")) {
            for (auto& c : a["components"].GetObject()) {
                std::string type = c.value["type"].GetString();
                if(type == "Rigidbody") {
                    // Create instance of component
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateRigidbody(c.name.GetString());
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);
                    
                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                }
                else if(type == "Animation") {
                    // Create instance of component
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateAnimation(c.name.GetString());
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);
                    
                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                }
                else if(ComponentDB::component_tables.find(type) != ComponentDB::component_tables.end()) { // Component exists
                    std::shared_ptr<luabridge::LuaRef> component_instance = ComponentDB::CreateComponentInstance(c.name.GetString(), type);
                    ComponentDB::OverrideComponentInstance(component_instance, c.value);

                    // Add to actor
                    new_actor.components[c.name.GetString()] = Component(c.name.GetString(), type, component_instance);
                } else {
                    std::cout << "error: failed to locate component " << type;
                    exit(0);
                }
            }
            // Add components to lifecycle queues
            for(auto& c : new_actor.components) {
                new_actor.onstart_queue.push_back(c.first);
                new_actor.update_queue.push_back(c.first);
                new_actor.lateupdate_queue.push_back(c.first);
            }
        }
    }
    return new_actor;
}

void Scene::Load(std::string scene_name) {
    if (std::filesystem::exists(resources / "scenes" / (scene_name + ".scene"))) {
        current_scene = scene_name;
        load_new = true;
    }
    else {
        std::cout << "error: scene " << scene_name << " is missing";
        exit(0);
    }
}

void Scene::LoadScene(std::string scene_name) {
    std::vector<Actor*> actors_temp;
    for(auto& a : actors) {
        if(a->donotdestroy) {
            actors_temp.push_back(a);
        } else {
            delete a;
        }
    }
    actors = actors_temp;
    n_actors = actors.size();
    load_new = false;
    rapidjson::Document scene;
    scene.SetNull();
    EngineUtils::ReadJsonFile((resources / "scenes" / (scene_name + ".scene")).generic_string(), scene);
    
    // Initialize actors
    for (auto& a : scene["actors"].GetArray()) {
        actors.push_back(new Actor(CreateActor(a)));
        n_actors++;
    }
    for (Actor* actor : actors) {
        for(auto& c : actor->components) {
            actor->InjectConvenienceReferences(c.second.componentRef);
        }
    }
}

std::string Scene::GetCurrent() {
    return current_scene;
}

void Scene::DontDestroy(Actor* actor) {
    actor->donotdestroy = true;
}

void Camera::SetPosition(float x, float y) {
    camera_pos = glm::vec2(x, y);
}
float Camera::GetPositionX() {
    return camera_pos.x;
}
float Camera::GetPositionY() {
    return camera_pos.y;
}
void Camera::SetZoom(float zoom) {
    zoom_factor = zoom;
}
float Camera::GetZoom() {
    return zoom_factor;
}

void DrawRequest::execute() {
   SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
   SDL_SetTextureAlphaMod(texture, color.a);
   if(layer_type > 0) {
       SDL_RenderSetScale(Scene::renderer, 1, 1);
   }
   Helper::SDL_RenderCopyEx498(0, "", Scene::renderer, texture, NULL, &rect, rotation, pivot, flip);
   SDL_SetTextureColorMod(texture, 255, 255, 255);
   SDL_SetTextureAlphaMod(texture, 255);
   render_queue.erase(render_queue.begin());
}
