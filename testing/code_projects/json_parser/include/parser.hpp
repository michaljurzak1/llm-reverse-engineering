#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <map>

class JsonParser {
private:
    std::map<std::string, std::string> data;
    
public:
    bool parseFile(const std::string& filename);
    std::string getValue(const std::string& key);
    void printAll();
};

#endif 