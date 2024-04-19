//
//  SceneDB.hpp
//  game_engine
//
//  Created by Jasmine Li on 2/3/24.
//

#ifndef SceneDB_hpp
#define SceneDB_hpp

#include <stdio.h>
#include <filesystem>
#include <optional>
#include <unordered_set>
#include <queue>
#include <SDL2/SDL.h>
#include "glm/glm.hpp"
#include "EngineUtils.h"
#include "Actor.hpp"
#include "lua.hpp"
#include "LuaBridge.h"

// Camera
extern int w;
extern int h;

extern Uint8 clear_color_r;
extern Uint8 clear_color_g;
extern Uint8 clear_color_b;

extern glm::ivec2 camera_offset;
extern float cam_ease_factor;

class Camera {
public:
    static inline glm::vec2 camera_pos = glm::vec2(0,0);
    static inline float zoom_factor = 1.0;
    static inline glm::ivec2 resolution = glm::ivec2(640, 360);
    static inline int ppu = 100;
    
    static void SetPosition(float x, float y);
    static float GetPositionX();
    static float GetPositionY();
    static void SetZoom(float zoom);
    static float GetZoom();
};

// Game
extern std::filesystem::path resources;
extern rapidjson::Document config;

extern std::string game_title;

extern std::string game_start_message;
extern std::string game_over_bad_message;
extern std::string game_over_good_message;

extern int n_actors;
extern std::vector<Actor*> actors;

class Scene {
public:
    static inline std::string current_scene;
    static inline bool load_new = false;
    static void LoadScene(std::string scene_name);
    
    static void Load(std::string scene_name);
    static std::string GetCurrent();
    static void DontDestroy(Actor* actor);
    
    static inline SDL_Window* window;
    static inline SDL_Renderer* renderer;
};

// Rendering

class IDrawRequest {
public:
    int layer_type;
    int sorting_order;
    
    IDrawRequest(int layer_type = 0, int sorting_order = 0)
            : layer_type(layer_type), sorting_order(sorting_order) {}
    
    virtual void execute() = 0;
    virtual ~IDrawRequest() {}
};

class DrawRequest : public IDrawRequest {
public:
    SDL_Texture* texture;
    SDL_Rect rect;
    SDL_Color color;
    float rotation;
    SDL_Point* pivot;
    SDL_RendererFlip flip;
    
    DrawRequest(int layer_type, int sorting_order, SDL_Texture* texture, SDL_Rect rect, SDL_Color color, float rotation, SDL_Point* pivot, SDL_RendererFlip flip) : IDrawRequest(layer_type, sorting_order), texture(texture), rect(rect), color(color), rotation(rotation), pivot(pivot), flip(flip) {}
    
    virtual void execute() override;
};

struct Pixel {
    int x;
    int y;
    int r;
    int g;
    int b;
    int a;
    
    Pixel(float x, float y, float r, float g, float b, float a) : x(x), y(y), r(r), g(g), b(b), a(a) {}
};
extern std::vector<IDrawRequest*> render_queue;
extern std::queue<Pixel> pixel_render_queue;

void LoadInitialScene();

void InitializeActors();
Actor CreateActor(rapidjson::Value& a);

#endif /* SceneDB_hpp */
