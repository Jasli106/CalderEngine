//
//  TextDB.hpp
//  game_engine
//
//  Created by Jasmine Li on 2/12/24.
//

#ifndef TextDB_hpp
#define TextDB_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "SDL2/SDL.h"
#include "SDL2_ttf/SDL_ttf.h"
#include "SceneDB.hpp"

class Text {
private:
    static inline std::unordered_map<std::string, std::unordered_map<int, TTF_Font*>> fonts;
public:
    static void Initialize();
    static void Draw(std::string str_content, float x, float y, std::string font_name, int font_size, float r, float g, float b, float a);
    
};

#endif /* TextDB_hpp */
