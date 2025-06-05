#include "parser.hpp"
#include "json_utils.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

bool JsonParser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    nlohmann::json j;
    file >> j;
    
    for (auto& [key, value] : j.items()) {
        data[key] = validateAndClean(value.dump());
    }
    return true;
}

std::string JsonParser::getValue(const std::string& key) {
    return data.count(key) ? data[key] : "";
}

void JsonParser::printAll() {
    for (const auto& [key, value] : data) {
        std::cout << key << ": " << value << std::endl;
    }
} 