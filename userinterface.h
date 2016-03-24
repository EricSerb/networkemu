#pragma once

#include <iostream>
#include <string>
#include <map>
#include <utility>

std::string parseCommands(char *buf);

typedef void (*printFunctions)(void);

