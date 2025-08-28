#include "interop.h"

char* duplicate_string(const char* str) {
#ifdef _WIN32
    return _strdup(str);
#else
    return strdup(str);
#endif
}