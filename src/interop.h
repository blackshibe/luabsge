#pragma once

#ifdef _WIN32
#include <string>
#else
#include <string.h>
#endif


char* duplicate_string(const char* str);