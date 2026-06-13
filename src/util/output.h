#pragma once

#include "include/colors.h"
#include <cstdarg>
#include <cstdio>
#include <source_location>

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

	void line(FILE *out, const char *color, const char *fmt, va_list args) const {
		fprintf(out, "%s[%s]%s ", color, name, ANSI_NC);
		vfprintf(out, fmt, args);
		fputc('\n', out);
	}

public:
	// default arg captures the declaration site's file, not log.h's
	Output(std::source_location loc = std::source_location::current()) : name(basename(loc.file_name())) {}

	void info(const char *fmt, ...) const { va_list a; va_start(a, fmt); line(stdout, ANSI_BLUE, fmt, a); va_end(a); }
	void warn(const char *fmt, ...) const { va_list a; va_start(a, fmt); line(stdout, ANSI_BOLD_YELLOW, fmt, a); va_end(a); }
	void error(const char *fmt, ...) const { va_list a; va_start(a, fmt); line(stderr, ANSI_BOLD_RED, fmt, a); va_end(a); }
};
