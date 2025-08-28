#include "config.h"

const char* concat_c_strings(const char* a, const char* b) {
	char *s = new char[strlen(a) + strlen(b) + 1];
	strcpy(s, a);
	strcat(s, b);

	return s;
}

BSGEConfig lua_bsge_get_config(sol::state &lua) {
    BSGEConfig config;
    
    sol::table bsge = lua["BSGE"];
    if (bsge.valid()) {
        sol::object default_asset_dir = bsge["default_asset_directory"];
        if (default_asset_dir.valid() && default_asset_dir.is<std::string>()) {
            config.default_asset_directory = default_asset_dir.as<std::string>();
        }
    }
    
    return config;
}