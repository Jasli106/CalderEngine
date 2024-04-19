//
//  main.cpp
//  game_engine
//
//  Created by Jasmine Li on 1/27/24.
//

#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <algorithm>
#include "EngineUtils.h"
#include "SceneDB.hpp"
#include "ImageDB.hpp"
#include "AudioDB.hpp"
#include "ComponentDB.hpp"
#include "Helper.h"
#include "Input.hpp"
#include "Rigidbody.hpp"
#include "Event.hpp"
#include "lua.hpp"
#include "LuaBridge.h"
#include "Animation.hpp"
 
bool playing = true;
bool waiting = false;
bool quit = false;
bool win = false;
bool intro = false;

int health = 3;
int score = 0;

int health_cooldown = -180;

std::unordered_set<int> scored;
std::unordered_map<uint64_t, std::vector<Actor*> > positions;

std::unordered_set<Actor*> nearby_sfx_triggered;

std::string next_scene = "";

int main(int argc, char * argv[]) {
    ComponentDB::Initialize();
    LoadInitialScene();

    std::string cmd;
    if (game_start_message != "") {
        std::cout << game_start_message << std::endl;
    }
    
    Audio::Initialize();
    Image::Initialize();
    Text::Initialize();
    
    Input::Init();
    while(playing) {
        SDL_Event inputEvent;
        while(Helper::SDL_PollEvent498(&inputEvent))
        {
            // Input
            Input::ProcessEvent(inputEvent);
            
            if(inputEvent.type == SDL_QUIT) {
                playing = false;
                quit = true;
            }
        }
        // Update
        for(auto &a : actors) {
            a->OnStart();
        }
        for(auto &a : actors) {
            a->Update();
        }
        Input::LateUpdate();
        for(auto &a : actors) {
            a->LateUpdate();
        }
        Actor::FrameEnd();
        
        EventManager::ProcessSubscriptions();
        EventManager::ProcessUnsubscriptions();
        
        // Physics step
        Physics::Step();
        
        // Render
        Image::Clear();
        // Add animations to render queue
        for(auto &pair : AnimationDB::to_render) {
            render_queue.push_back(new AnimationDrawRequest(0, 0, pair.second));
        }
        // Sort render queue
        std::stable_sort(render_queue.begin(), render_queue.end(), [](const IDrawRequest* a, const IDrawRequest* b) {
            if (a->layer_type != b->layer_type) {
                return a->layer_type < b->layer_type;
            }
            return a->sorting_order < b->sorting_order;
        });
        SDL_RenderSetScale(Scene::renderer, Camera::zoom_factor, Camera::zoom_factor);
        while(!render_queue.empty()) {
            IDrawRequest* r = render_queue.front();
            r->execute();
            render_queue.erase(render_queue.begin());
        }
        SDL_SetRenderDrawBlendMode(Scene::renderer, SDL_BLENDMODE_BLEND);
        while(!pixel_render_queue.empty()) {
            Pixel p = pixel_render_queue.front();
            SDL_SetRenderDrawColor(Scene::renderer, p.r, p.g, p.b, p.a);
            SDL_RenderDrawPoint(Scene::renderer, p.x, p.y);
            pixel_render_queue.pop();
        }
        Helper::SDL_RenderPresent498(Scene::renderer);
        
        if(Scene::load_new) {
            Scene::LoadScene(Scene::current_scene);
        }
    }
    if(quit) {
        return 0;
    }
    
    return 0;
}
