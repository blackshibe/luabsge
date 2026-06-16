#pragma once

#include "include/colors.h"
#include <cstdarg>
#include <cstdio>
#include <source_location>

enum class LogDomain {
	Engine,
	Vulkan,
	Count,
};

enum class LogLevel {
	Trace,
	Info,
	Warn,
	Error,
	Off,
};

struct DomainConfig {
	bool enabled = true;
	LogLevel min_level = LogLevel::Trace;
};

// per-domain runtime configuration. defaults: every domain enabled at Trace.
inline DomainConfig &log_domain_config(LogDomain domain) {
	static DomainConfig configs[(int)LogDomain::Count];
	return configs[(int)domain];
}

inline const char *log_domain_name(LogDomain domain) {
	switch (domain) {
	case LogDomain::Engine:
		return "engine";
	case LogDomain::Vulkan:
		return "vulkan";
	default:
		return "?";
	}
}

// source location needs to be evaluated at the call site so this exists
// literal magic lol
struct LogFormat {
	const char *fmt;
	std::source_location location;
	LogFormat(const char *fmt, std::source_location location = std::source_location::current()) : fmt(fmt), location(location) {}
};

class Output {
	const char *name;
	LogDomain domain;

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

	bool enabled(LogLevel level) const {
		const DomainConfig &config = log_domain_config(domain);
		return config.enabled && (int)level >= (int)config.min_level;
	}

	void line(FILE *out, const char *color, LogLevel level, LogFormat fmt, va_list args) const {
		if (!enabled(level)) return;

		fprintf(out, "%s[%s][%s:%u]%s ", color, log_domain_name(domain), name, (unsigned)fmt.location.line(), ANSI_NC);
		vfprintf(out, fmt.fmt, args);
		fputc('\n', out);
	}

  public:
	// default arg captures the declaration site's file, not log.h's
	Output(LogDomain domain = LogDomain::Engine, std::source_location location = std::source_location::current())
	    : name(basename(location.file_name())), domain(domain) {}

	static void set_enabled(LogDomain domain, bool enabled) {
		log_domain_config(domain).enabled = enabled;
	}

	static void set_level(LogDomain domain, LogLevel level) {
		log_domain_config(domain).min_level = level;
	}

	void trace(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stdout, ANSI_GREEN, LogLevel::Trace, fmt, a);
		va_end(a);
	}

	void info(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stdout, ANSI_BLUE, LogLevel::Info, fmt, a);
		va_end(a);
	}

	void warn(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stdout, ANSI_BOLD_YELLOW, LogLevel::Warn, fmt, a);
		va_end(a);
	}

	void error(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stderr, ANSI_BOLD_RED, LogLevel::Error, fmt, a);
		va_end(a);
	}

	void comment(LogFormat fmt, ...) const {
		va_list a;
		va_start(a, fmt);
		line(stderr, ANSI_GREEN, LogLevel::Trace, fmt, a);
		va_end(a);
	}

	void mark(std::source_location location = std::source_location::current()) const {
		comment("%s", location.function_name());
	}
};
