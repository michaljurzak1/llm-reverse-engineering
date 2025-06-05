#include "parser.hpp"
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description desc("JSON Parser Options");
    desc.add_options()
        ("help,h", "Show help message")
        ("file,f", po::value<std::string>(), "JSON file to parse")
        ("key,k", po::value<std::string>(), "Key to extract");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    
    JsonParser parser;
    if (vm.count("file")) {
        std::string filename = vm["file"].as<std::string>();
        if (parser.parseFile(filename)) {
            if (vm.count("key")) {
                std::string key = vm["key"].as<std::string>();
                std::cout << parser.getValue(key) << std::endl;
            } else {
                parser.printAll();
            }
        } else {
            std::cerr << "Failed to parse file: " << filename << std::endl;
            return 1;
        }
    }
    
    return 0;
} 