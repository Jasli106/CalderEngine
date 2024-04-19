//
//  AudioDB.cpp
//  game_engine
//
//  Created by Jasmine Li on 2/13/24.
//

#include "AudioDB.hpp"

void Audio::Initialize() {
    AudioHelper::Mix_OpenAudio498(44100, AUDIO_S16SYS, 2, 2048);
    AudioHelper::Mix_AllocateChannels498(50);
}

void Audio::Play(int channel, std::string clip_name, bool loops) {
    if(loaded_audio.find(clip_name) == loaded_audio.end()) {
        std::filesystem::path wav = resources/"audio"/(clip_name +".wav");
        std::filesystem::path ogg = resources/"audio"/(clip_name +".ogg");
        if(std::filesystem::exists(wav)) {
            loaded_audio[clip_name] = AudioHelper::Mix_LoadWAV498(wav.string().c_str());
        } else if(std::filesystem::exists(ogg)) {
            loaded_audio[clip_name] = AudioHelper::Mix_LoadWAV498(ogg.string().c_str());
        } else {
            std::cout << "error: failed to play audio clip " + clip_name;
            exit(0);
        }
    }
    int l = 0;
    if(loops) {
        l = -1;
    }
    AudioHelper::Mix_PlayChannel498(channel, loaded_audio[clip_name], l);
}

void Audio::Halt(int channel) {
    AudioHelper::Mix_HaltChannel498(channel);
}

void Audio::SetVolume(int channel, int volume) {
    AudioHelper::Mix_Volume498(channel, volume);
}
