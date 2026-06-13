#pragma once

#include "../web/download_image.h"
#include "class/image.h"

#include <sol/sol.hpp>
#include <unordered_map>
#include <string>

struct Emscripten {};

extern std::unordered_map<std::string, int> emscripten_int_storage;

extern "C" {
    void js_set_int(const char* key, int value);
    int js_get_int(const char* key);
}

int emscripten_get_int(const std::string& key);
void emscripten_set_int(const std::string& key, int value);

void lua_bsge_init_emscripten(sol::state &lua);