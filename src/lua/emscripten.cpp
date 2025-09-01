#include "emscripten.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

std::unordered_map<std::string, int> emscripten_int_storage;

bsgeImage download_image(sol::this_state lua, const char* src) {
    return bsgeImage(lua, src, bsgeImageLoadSource::web);
}

extern "C" {
    void js_set_int(const char* key, int value) {
        emscripten_int_storage[std::string(key)] = value;
    }
    
    int js_get_int(const char* key) {
        auto it = emscripten_int_storage.find(std::string(key));
        return (it != emscripten_int_storage.end()) ? it->second : 0;
    }
}

int emscripten_get_int(const std::string& key) {
    auto it = emscripten_int_storage.find(key);
    return (it != emscripten_int_storage.end()) ? it->second : 0;
}

void emscripten_set_int(const std::string& key, int value) {
    emscripten_int_storage[key] = value;
}

void lua_bsge_init_emscripten(sol::state &lua) {
	lua.new_usertype<Emscripten>("Emscripten",
                                "download_image", &download_image,
                                "get_int", &emscripten_get_int,
                                "set_int", &emscripten_set_int
                                );
}
