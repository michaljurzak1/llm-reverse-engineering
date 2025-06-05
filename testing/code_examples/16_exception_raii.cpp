#include <iostream>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

// Custom exception classes
class ResourceException : public std::runtime_error {
public:
    explicit ResourceException(const std::string& message)
        : std::runtime_error("Resource error: " + message) {}
};

class ValidationException : public std::runtime_error {
public:
    explicit ValidationException(const std::string& message)
        : std::runtime_error("Validation error: " + message) {}
};

// RAII wrapper for file handling
class FileHandler {
private:
    std::fstream file;
    std::string filename;

public:
    explicit FileHandler(const std::string& name) : filename(name) {
        file.open(name, std::ios::in | std::ios::out | std::ios::app);
        if (!file.is_open()) {
            throw ResourceException("Failed to open file: " + name);
        }
    }

    ~FileHandler() {
        if (file.is_open()) {
            file.close();
        }
    }

    void write(const std::string& data) {
        if (!file.is_open()) {
            throw ResourceException("File not open: " + filename);
        }
        file << data << std::endl;
        if (file.fail()) {
            throw ResourceException("Failed to write to file: " + filename);
        }
    }

    std::string read() {
        if (!file.is_open()) {
            throw ResourceException("File not open: " + filename);
        }
        std::string line;
        std::getline(file, line);
        if (file.fail() && !file.eof()) {
            throw ResourceException("Failed to read from file: " + filename);
        }
        return line;
    }
};

// RAII wrapper for memory management
template<typename T>
class ScopedArray {
private:
    T* ptr;
    size_t size;

public:
    ScopedArray(size_t n) : size(n) {
        ptr = new T[n];
        if (!ptr) {
            throw ResourceException("Failed to allocate memory");
        }
    }

    ~ScopedArray() {
        delete[] ptr;
    }

    T& operator[](size_t index) {
        if (index >= size) {
            throw ValidationException("Index out of bounds");
        }
        return ptr[index];
    }

    size_t getSize() const { return size; }
};

// RAII wrapper for transaction-like operations
class Transaction {
private:
    bool committed;
    std::vector<std::function<void()>> rollback_actions;

public:
    Transaction() : committed(false) {}

    ~Transaction() {
        if (!committed) {
            rollback();
        }
    }

    void addRollbackAction(std::function<void()> action) {
        rollback_actions.push_back(action);
    }

    void commit() {
        committed = true;
    }

    void rollback() {
        for (auto it = rollback_actions.rbegin(); it != rollback_actions.rend(); ++it) {
            try {
                (*it)();
            } catch (const std::exception& e) {
                std::cerr << "Rollback action failed: " << e.what() << std::endl;
            }
        }
    }
};

// Example class using RAII
class DataProcessor {
private:
    std::unique_ptr<FileHandler> file;
    ScopedArray<int> data;
    Transaction transaction;

public:
    DataProcessor(const std::string& filename, size_t size)
        : file(std::make_unique<FileHandler>(filename)), data(size) {
        // Register rollback action
        transaction.addRollbackAction([this]() {
            file->write("Rollback: Data processing failed");
        });
    }

    void processData() {
        try {
            // Simulate data processing
            for (size_t i = 0; i < data.getSize(); ++i) {
                data[i] = i * 2;
                if (data[i] > 100) {
                    throw ValidationException("Data value too large");
                }
            }

            // Write processed data
            for (size_t i = 0; i < data.getSize(); ++i) {
                file->write(std::to_string(data[i]));
            }

            // Commit the transaction
            transaction.commit();
        } catch (const std::exception& e) {
            std::cerr << "Processing failed: " << e.what() << std::endl;
            throw; // Re-throw to trigger rollback
        }
    }
};

// Function demonstrating exception handling
void demonstrateExceptionHandling() {
    try {
        // Create a data processor with a small array
        DataProcessor processor("data.txt", 5);
        processor.processData();
    } catch (const ValidationException& e) {
        std::cerr << "Validation error occurred: " << e.what() << std::endl;
    } catch (const ResourceException& e) {
        std::cerr << "Resource error occurred: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error occurred: " << e.what() << std::endl;
    }
}

// Function demonstrating stack unwinding
void demonstrateStackUnwinding() {
    std::cout << "Starting stack unwinding demonstration..." << std::endl;
    
    try {
        // Create nested scopes to demonstrate stack unwinding
        {
            FileHandler file1("file1.txt");
            file1.write("Data in file1");
            
            {
                FileHandler file2("file2.txt");
                file2.write("Data in file2");
                
                // This will cause an exception
                throw ValidationException("Intentional exception for demonstration");
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    
    std::cout << "Stack unwinding demonstration completed." << std::endl;
}

int main() {
    std::cout << "Demonstrating exception handling with RAII..." << std::endl;
    demonstrateExceptionHandling();
    
    std::cout << "\nDemonstrating stack unwinding..." << std::endl;
    demonstrateStackUnwinding();
    
    return 0;
} 