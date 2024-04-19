//
//  AudioDB.hpp
//  game_engine
//
//  Created by Jasmine Li on 2/13/24.
//

#ifndef AudioDB_hpp
#define AudioDB_hpp

#include <stdio.h>
#include <vector>
#include "SceneDB.hpp"
#include "AudioHelper.h"

class Audio {
private:
    static inline std::unordered_map<std::string, Mix_Chunk*> loaded_audio;
public:
    static void Initialize();
    static void Play(int channel, std::string clip_name, bool loops);
    static void Halt(int channel);
    static void SetVolume(int channel, int volume);
};

#endif /* AudioDB_hpp */
