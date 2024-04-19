//
//  TextDB.cpp
//  game_engine
//
//  Created by Jasmine Li on 2/12/24.
//

#include "TextDB.hpp"
#include "ImageDB.hpp"

void Text::Initialize() {
    TTF_Init();
}

void Text::Draw(std::string str_content, float x, float y, std::string font_name, int font_size, float r, float g, float b, float a) {
    TTF_Font* font;
    if(fonts.find(font_name) != fonts.end()) {
        if(fonts[font_name].find(font_size) != fonts[font_name].end()) {
            font = fonts[font_name][font_size];
        } else {
            std::filesystem::path path = resources/"fonts"/(font_name +".ttf");
            if(!std::filesystem::exists(path)) {
                std::cout << "error: font " + font_name + " missing";
                exit(0);
            }
            font = TTF_OpenFont(path.string().c_str(), font_size);
        }
    } else {
        std::filesystem::path path = resources/"fonts"/(font_name +".ttf");
        if(!std::filesystem::exists(path)) {
            std::cout << "error: font " + font_name + " missing";
            exit(0);
        }
        font = TTF_OpenFont(path.string().c_str(), font_size);
    }
    SDL_Color color = {static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a)};
    SDL_Surface* surface = TTF_RenderText_Solid(font, str_content.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(Scene::renderer, surface);
    SDL_Rect rect = {static_cast<int>(x), static_cast<int>(y), surface->w, surface->h};
    SDL_FreeSurface(surface);
    SDL_Color color_mod = {255, 255, 255, 255};
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    SDL_Point pivot = {static_cast<int>(0.5f * width), static_cast<int>(0.5f * height)};
    render_queue.push_back(new DrawRequest(2, 0, texture, rect, color_mod, 0, &pivot, SDL_FLIP_NONE));
}
