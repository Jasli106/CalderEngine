//
//  ImageDB.hpp
//  game_engine
//
//  Created by Jasmine Li on 2/8/24.
//

#ifndef ImageDB_hpp
#define ImageDB_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "Helper.h"
#include "SceneDB.hpp"
#include "TextDB.hpp"


class Image {
public:
    static inline std::unordered_map<std::string, SDL_Texture*> loaded_imgs;
    
    static void Initialize();
    static void Clear();
    static void DrawUI(std::string image_name, float x, float y);
    static void DrawUIEx(std::string image_name, float x, float y, float r, float g, float b, float a, float sorting_order);
    static void Draw(std::string image_name, float x, float y);
    static void DrawEx(std::string image_name, float x, float y, float rot_deg, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, float sorting_order);
    static void DrawPixel(float x, float y, float r, float g, float b, float a);
};

#endif /* ImageDB_hpp */
