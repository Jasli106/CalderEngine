//
//  EngineUtils.h
//  game_engine
//
//  Created by Jasmine Li on 1/30/24.
//

#ifndef EngineUtils_h
#define EngineUtils_h

#include <iostream>
#include "SDL2/SDL.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

class EngineUtils {
public:

    /* Code provided from EECS 498.007 staff */
    static void ReadJsonFile(const std::string& path, rapidjson::Document & out_document)
    {
        FILE* file_pointer = nullptr;
    #ifdef _WIN32
        fopen_s(&file_pointer, path.c_str(), "rb");
    #else
        file_pointer = fopen(path.c_str(), "rb");
    #endif
        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);
        std::fclose(file_pointer);

        if (out_document.HasParseError()) {
            rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
            std::cout << errorCode << "error parsing json at [" << path << "]" << std::endl;
            exit(0);
        }
    }

    /* Code provided from EECS 498.007 staff */
    static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase) {
        size_t pos = input.find(phrase);

        if (pos == std::string::npos) return "";

        pos += phrase.length();
        while (pos < input.size() && std::isspace(input[pos])) {
            ++pos;
        }

        if (pos == input.size()) return "";

        size_t endPos = pos;
        while (endPos < input.size() && !std::isspace(input[endPos])) {
            ++endPos;
        }

        return input.substr(pos, endPos - pos);
    }

    /* Code provided from EECS 498.007 staff */
    static uint64_t create_composite_key(int x, int y) {
        uint32_t ux = static_cast<uint32_t>(x);
        uint32_t uy = static_cast<uint32_t>(y);

        uint64_t result = static_cast<uint64_t>(ux);

        result = result << 32;

        result = result | static_cast<uint64_t>(uy);

        return result;
    }
    
    /* Code provided from EECS 498.007 staff */
    static SDL_RendererFlip GetRendererFlip(bool horizontalFlip, bool verticalFlip)
    {
        int flip = 0;
        if (horizontalFlip)
        {
            flip |= SDL_FLIP_HORIZONTAL;
        }
        if (verticalFlip)
        {
            flip |= SDL_FLIP_VERTICAL;
        }
        return static_cast<SDL_RendererFlip>(flip);
    }

};

#endif /* EngineUtils_h */
