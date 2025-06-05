# JSON Parser Project

A C++ command-line tool for parsing and extracting data from JSON files. This project demonstrates advanced C++ features including external library integration, class design, and command-line argument parsing.

## Prerequisites

Before building the project, ensure you have the following dependencies installed:

### Ubuntu/Debian

```bash
# Install Boost libraries
sudo apt-get update
sudo apt-get install libboost-all-dev

# Install nlohmann/json library
sudo apt-get install nlohmann-json3-dev
```

### CentOS/RHEL

```bash
# Install Boost libraries
sudo yum install boost-devel

# Install nlohmann/json library
sudo yum install nlohmann-json-devel
```

### macOS

```bash
# Install Boost libraries
brew install boost

# Install nlohmann/json library
brew install nlohmann-json
```

## Building the Project

1. Clone the repository (if applicable)
2. Navigate to the project directory:

```bash
cd testing/code_projects/json_parser
```

3. Build the project:

```bash
make clean
make
```

The executable will be created in the `build` directory as `json_app`.

## Usage

The program supports the following command-line options:

```bash
./build/json_app [options]
```

### Options:

- `-h, --help`: Show help message
- `-f, --file <filename>`: Specify the JSON file to parse
- `-k, --key <key>`: Extract a specific key from the JSON file

### Examples:

1. Show help message:

```bash
./build/json_app --help
```

2. Parse entire JSON file:

```bash
./build/json_app --file test.json
```

3. Extract specific key:

```bash
./build/json_app --file test.json --key name
```

## Sample JSON File

A sample JSON file (`test.json`) is included in the project:

```json
{
  "name": "John Doe",
  "age": 30,
  "email": "john@example.com",
  "address": {
    "street": "123 Main St",
    "city": "Boston",
    "zip": "02108"
  },
  "hobbies": ["reading", "gaming", "coding"],
  "is_active": true
}
```

## Project Structure

```
json_parser/
├── src/
│   ├── main.cpp          # Main program entry point
│   ├── parser.cpp        # JSON parser implementation
│   └── json_utils.cpp    # JSON utility functions
├── include/
│   ├── parser.hpp        # Parser class declaration
│   └── json_utils.hpp    # Utility function declarations
├── external/             # External dependencies
├── build/               # Build output directory
└── Makefile            # Build configuration
```

## Features

- JSON file parsing and validation
- Command-line argument parsing using Boost.ProgramOptions
- Support for nested JSON structures
- Error handling for file operations and parsing
- Clean and modular code structure

## Error Handling

The program handles various error conditions:

- Missing or invalid JSON file
- Invalid JSON format
- Missing or invalid keys
- File access permissions

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a new Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
