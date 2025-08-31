#include "emscripten.h"
#include <iostream>

bsgeImage download_image(sol::this_state lua, const char* src) {
    return bsgeImage(lua, src, bsgeImageLoadSource::web);
}

void lua_bsge_init_emscripten(sol::state &lua) {
	lua.new_usertype<Emscripten>("Emscripten",
                                "download_image", &download_image
                                );
}
