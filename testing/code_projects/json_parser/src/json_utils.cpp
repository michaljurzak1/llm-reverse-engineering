#include "json_utils.hpp"
#include <algorithm>
#include <regex>

std::string validateAndClean(const std::string& jsonStr) {
    std::string cleaned = jsonStr;
    // Remove quotes for simple values
    if (cleaned.front() == '"' && cleaned.back() == '"') {
        cleaned = cleaned.substr(1, cleaned.length() - 2);
    }
    return cleaned;
}

bool isValidJson(const std::string& str) {
    // Simple JSON validation
    std::regex jsonRegex(R"(^[\{\[].*[\}\]]$|^".*"$|^[0-9.-]+$|^(true|false|null)$)");
    return std::regex_match(str, jsonRegex);
} 