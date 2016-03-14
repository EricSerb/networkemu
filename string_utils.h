#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, char delim, int maxChunks = -1);
std::string trim(const std::string &s, const std::string &delims);

#endif