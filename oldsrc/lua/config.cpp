#include "config.h"

const char* concat_c_strings(const char* a, const char* b) {
	char *s = new char[strlen(a) + strlen(b) + 1];
	strcpy(s, a);
	strcat(s, b);

	return s;
}

BSGEConfig lua_bsge_get_config(sol::state &lua) {
    BSGEConfig config;
    config.default_asset_directory = ""; // Default to empty string
    
    sol::optional<sol::table> bsge_opt = lua["BSGE"];
    if (bsge_opt.has_value()) {
        sol::table bsge = bsge_opt.value();
        sol::optional<std::string> default_asset_dir_opt = bsge["default_asset_directory"];
        if (default_asset_dir_opt.has_value()) {
            config.default_asset_directory = default_asset_dir_opt.value();
        }
    }
    
    return config;
}