#ifndef JSON_UTILS_HPP
#define JSON_UTILS_HPP

#include <string>

std::string validateAndClean(const std::string& jsonStr);
bool isValidJson(const std::string& str);

#endif 