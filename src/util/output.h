#pragma once

#include "include/colors.h"
#include <cstdarg>
#include <cstdio>
#include <source_location>

// source location needs to be evaluated at the call site so this exists
// literal magic lol
struct LogFormat {
	const char *fmt;
	std::source_location location;
	LogFormat(const char *fmt, std::source_location location = std::source_location::current()) : fmt(fmt), location(location) {}
};

class Output {
	const char *name;

	// full path -> just the file name
	static constexpr const char *basename(const char *path) {
		const char *file = path;
		while (*path) {
			if (*path == '/' || *path == '\\')
				file = path + 1;
			++path;
		}
		return file;
	}

	void line(FILE *out, const char *color, LogFormat fmt, va_list args) const {
		fprintf(out, "%s[%s:%u]%s ", color, name, (unsigned)fmt.location.line(), ANSI_NC);
		vfprintf(out, fmt.fmt, args);
		fputc('\n', out);
	}

  public:
	// default arg captures the declaration site's file, not log.h's
	Output(std::source_location location = std::source_location::current()) : name(basename(location.file_name())) {}

	void info(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stdout, ANSI_BLUE, fmt, a);
		va_end(a);
	}

	void warn(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stdout, ANSI_BOLD_YELLOW, fmt, a);
		va_end(a);
	}

	void error(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stderr, ANSI_BOLD_RED, fmt, a);
		va_end(a);
	}

	void mark(std::source_location location = std::source_location::current()) const {
		fprintf(stdout, "%s[%s] %s%s\n", ANSI_GREEN, name, location.function_name(), ANSI_NC);
	}
};
