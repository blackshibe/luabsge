#include "util/time.h"

#include <chrono>

using namespace std::chrono;

namespace Lua::time {
	int64_t now() {
		int64_t ms = duration_cast<milliseconds>(
						 system_clock::now().time_since_epoch())
						 .count();
		return ms;
	}
}
