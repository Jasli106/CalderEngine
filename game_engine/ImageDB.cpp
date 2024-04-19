//
//  ImageDB.cpp
//  game_engine
//
//  Created by Jasmine Li on 2/8/24.
//

#include "ImageDB.hpp"

void Image::Clear() {
    SDL_SetRenderDrawColor(Scene::renderer, clear_color_r, clear_color_g, clear_color_b, 255);
    SDL_RenderClear(Scene::renderer);
}

void Image::Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    // Create window
    Scene::window = Helper::SDL_CreateWindow498(game_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Camera::resolution.x, Camera::resolution.y, 0);
    Scene::renderer = Helper::SDL_CreateRenderer498(Scene::window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    Clear();
    Scene::Load(config["initial_scene"].GetString());
    Scene::LoadScene(config["initial_scene"].GetString());
}

void Image::DrawUI(std::string image_name, float x, float y) {
    if(loaded_imgs.find(image_name) == loaded_imgs.end()) {
        std::filesystem::path path = resources/"images"/(image_name +".png");
        if(!std::filesystem::exists(path)) {
            std::cout << "error: missing image " + image_name;
            exit(0);
        }
        loaded_imgs[image_name] = IMG_LoadTexture(Scene::renderer, path.string().c_str());
    }
    SDL_Texture* texture = loaded_imgs[image_name];
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect rect = {static_cast<int>(x), static_cast<int>(y), width, height};
    SDL_Color color = {255, 255, 255, 255};
    SDL_Point pivot = {static_cast<int>(0.5f * width), static_cast<int>(0.5f * height)};
    render_queue.push_back(new DrawRequest(1, 0, texture, rect, color, 0, &pivot, SDL_FLIP_NONE));
}

void Image::DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order) {
    if(loaded_imgs.find(image_name) == loaded_imgs.end()) {
        std::filesystem::path path = resources/"images"/(image_name +".png");
        if(!std::filesystem::exists(path)) {
            std::cout << "error: missing image " + image_name;
            exit(0);
        }
        loaded_imgs[image_name] = IMG_LoadTexture(Scene::renderer, path.string().c_str());
    }
    SDL_Texture* texture = loaded_imgs[image_name];
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Rect rect = {static_cast<int>(x), static_cast<int>(y), width, height};
    SDL_Color color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};
    SDL_Point pivot = {static_cast<int>(0.5f * width), static_cast<int>(0.5f * height)};
    render_queue.push_back(new DrawRequest(1, sorting_order, texture, rect, color, 0, &pivot, SDL_FLIP_NONE));
}

void Image::Draw(std::string image_name, float x, float y) {
    if(loaded_imgs.find(image_name) == loaded_imgs.end()) {
        std::filesystem::path path = resources/"images"/(image_name +".png");
        if(!std::filesystem::exists(path)) {
            std::cout << "error: missing image " + image_name;
            exit(0);
        }
        loaded_imgs[image_name] = IMG_LoadTexture(Scene::renderer, path.string().c_str());
    }
    SDL_Texture* texture = loaded_imgs[image_name];
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Color color = {255, 255, 255, 255};
    glm::vec2 cam_adj_pos = glm::vec2(x, y) - Camera::camera_pos;
    SDL_Point* pivot = new SDL_Point({static_cast<int>(0.5f * width), static_cast<int>(0.5f * height)});
    int finalX = static_cast<int>(cam_adj_pos.x * Camera::ppu + Camera::resolution.x * 0.5f * (1.0f/Camera::zoom_factor) - pivot->x);
    int finalY = static_cast<int>(cam_adj_pos.y * Camera::ppu + Camera::resolution.y * 0.5f * (1.0f/Camera::zoom_factor) - pivot->y);
    SDL_Rect rect = {
        finalX, finalY,
        width , height
    };
    render_queue.push_back(new DrawRequest(0, 0, texture, rect, color, 0, pivot, SDL_FLIP_NONE));
}

void Image::DrawEx(std::string image_name, float x, float y, float rot_deg, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order) {
    if(loaded_imgs.find(image_name) == loaded_imgs.end()) {
        std::filesystem::path path = resources/"images"/(image_name +".png");
        if(!std::filesystem::exists(path)) {
            std::cout << "error: missing image " + image_name;
            exit(0);
        }
        loaded_imgs[image_name] = IMG_LoadTexture(Scene::renderer, path.string().c_str());
    }
    SDL_Texture* texture = loaded_imgs[image_name];
    SDL_Color color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};
    glm::vec2 cam_adj_pos = glm::vec2(x, y) - Camera::camera_pos;
    SDL_Rect rect;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    rect.w *= std::abs(scale_x);
    rect.h *= std::abs(scale_y);
    SDL_Point* pivot = new SDL_Point({static_cast<int>(pivot_x * rect.w), static_cast<int>(pivot_y * rect.h)});
    rect.x = static_cast<int>(cam_adj_pos.x * Camera::ppu + Camera::resolution.x * 0.5f * (1.0f/Camera::zoom_factor) - pivot->x);
    rect.y = static_cast<int>(cam_adj_pos.y * Camera::ppu + Camera::resolution.y * 0.5f * (1.0f/Camera::zoom_factor) - pivot->y);
    render_queue.push_back(new DrawRequest(0, sorting_order, texture, rect, color, static_cast<int>(rot_deg), pivot, EngineUtils::GetRendererFlip(scale_x < 0, scale_y < 0)));
}

void Image::DrawPixel(float x, float y, float r, float g, float b, float a) {
    pixel_render_queue.push(Pixel(x, y, r, g, b, a));
}
