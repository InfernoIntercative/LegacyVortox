#pragma once

#include "../core/core.hpp"

extern int total_messages;

void error(const char *message, const char *log = nullptr);
void debug(const char *message, const char *log = nullptr);
void okay(const char *message, const char *log = nullptr);
void info(const char *message, const char *log = nullptr);
void warn(const char *message, const char *log = nullptr);
void panic(const char *message, const char *log = nullptr);
void summary();