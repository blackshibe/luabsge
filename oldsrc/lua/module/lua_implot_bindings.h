#pragma once

#include "../lua_state.h"
#include <lua.hpp>

#include "../../include/implot/implot.h"

void lua_bsge_init_implot_bindings(sol::state &lua);