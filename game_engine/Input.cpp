#include "Input.hpp"
#include "SDL2/SDL.h"
#include <iostream>
#include <unordered_map>
#include <vector>

/* Code provided in EECS 498.007 W24 Lecture 9 */
void Input::Init() {
    for(int code= SDL_SCANCODE_UNKNOWN; code < SDL_NUM_SCANCODES; ++code) {
        keyboard_states[static_cast<SDL_Scancode>(code)] = INPUT_STATE_UP;
    }
    for(int i=1; i<4; ++i) {
        mouse_button_states[i] = INPUT_STATE_UP;
    }
}
void Input::ProcessEvent(const SDL_Event& e) {
    if(e.type == SDL_KEYDOWN) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_scancodes.push_back(e.key.keysym.scancode);
    }
    else if(e.type == SDL_KEYUP) {
        keyboard_states[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_scancodes.push_back(e.key.keysym.scancode);
    }
    else if (e.type == SDL_MOUSEMOTION) {
        mouse_position.x = e.motion.x;
        mouse_position.y = e.motion.y;
    }
    else if (e.type == SDL_MOUSEBUTTONDOWN) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
        just_became_down_buttons.push_back(e.button.button);
    }
    else if (e.type == SDL_MOUSEBUTTONUP) {
        mouse_button_states[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
        just_became_up_buttons.push_back(e.button.button);
    }
    else if(e.type == SDL_MOUSEWHEEL) {
        mouse_scroll_this_frame = e.wheel.preciseY;
    }
}
void Input::LateUpdate() {
    for(const SDL_Scancode& code : just_became_down_scancodes) {
        keyboard_states[code] = INPUT_STATE_DOWN;
    }
    just_became_down_scancodes.clear();
    
    for(const SDL_Scancode& code : just_became_up_scancodes) {
        keyboard_states[code] = INPUT_STATE_UP;
    }
    just_became_up_scancodes.clear();
    
    for(const int& idx : just_became_down_buttons) {
        mouse_button_states[idx] = INPUT_STATE_DOWN;
    }
    just_became_down_buttons.clear();
    
    for(const int& idx : just_became_up_buttons) {
        mouse_button_states[idx] = INPUT_STATE_UP;
    }
    just_became_up_buttons.clear();
    mouse_scroll_this_frame = 0.0;
}

bool Input::GetKey(std::string keycode) {
    try {
        SDL_Scancode k = __keycode_to_scancode.at(keycode);
        return keyboard_states[k] == INPUT_STATE_DOWN || keyboard_states[k] == INPUT_STATE_JUST_BECAME_DOWN;
    }
    catch (const std::out_of_range &e){
        return false;
    }
}
bool Input::GetKeyDown(std::string keycode) {
    try {
        SDL_Scancode k = __keycode_to_scancode.at(keycode);
        return keyboard_states[k] == INPUT_STATE_JUST_BECAME_DOWN;
    }
    catch (const std::out_of_range &e){
        return false;
    }
}
bool Input::GetKeyUp(std::string keycode) {
    try {
        SDL_Scancode k = __keycode_to_scancode.at(keycode);
        return keyboard_states[k] == INPUT_STATE_JUST_BECAME_UP;
    }
    catch (const std::out_of_range &e){
        return false;
    }
}

glm::vec2 Input::GetMousePosition() {
    return mouse_position;
}
bool Input::GetMouseButton(int button) {
    if(button > 0 && button < 4) {
        return mouse_button_states[button] == INPUT_STATE_DOWN || mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
    }
    return false;
}
bool Input::GetMouseButtonDown(int button) {
    if(button > 0 && button < 4) {
        return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_DOWN;
    }
    return false;
}
bool Input::GetMouseButtonUp(int button) {
    if(button > 0 && button < 4) {
        return mouse_button_states[button] == INPUT_STATE_JUST_BECAME_UP;
    }
    return false;
}
float Input::GetMouseScrollDelta() {
    return mouse_scroll_this_frame;
}
