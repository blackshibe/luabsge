
#pragma once
#include <string>
#include <sol/sol.hpp>

struct BSGEConfig {
    std::string default_asset_directory;

    BSGEConfig() {
        default_asset_directory = "";
    }
};


BSGEConfig lua_bsge_get_config(sol::state &lua);
const char* concat_c_strings(const char* a, const char* b);