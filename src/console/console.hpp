#pragma once

#include <stdio.h>
#include <string>
#include <vector>

extern bool showConsole;
extern std::string inputCommand;
extern std::vector<std::string> history;
extern int historyIndex;

void processCommand(const std::string &command);
void renderConsole();
