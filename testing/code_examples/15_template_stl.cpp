#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <algorithm>
#include <numeric>
#include <functional>
#include <string>
#include <memory>
#include <type_traits>

// Template class for a generic container wrapper
template<typename Container>
class ContainerWrapper {
private:
    Container data;

public:
    // Type aliases for container types
    using value_type = typename Container::value_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

    // Constructor with variadic arguments
    template<typename... Args>
    ContainerWrapper(Args&&... args) : data(std::forward<Args>(args)...) {}

    // Generic insert method
    template<typename T>
    void insert(const T& value) {
        if constexpr (std::is_same_v<Container, std::set<typename Container::value_type>>) {
            data.insert(value);
        } else if constexpr (std::is_same_v<Container, std::map<typename Container::key_type, typename Container::mapped_type>>) {
            data.insert(value);
        } else {
            data.push_back(value);
        }
    }

    // Generic find method
    template<typename T>
    iterator find(const T& value) {
        if constexpr (std::is_same_v<Container, std::set<typename Container::value_type>> ||
                     std::is_same_v<Container, std::map<typename Container::key_type, typename Container::mapped_type>>) {
            return data.find(value);
        } else {
            return std::find(data.begin(), data.end(), value);
        }
    }

    // Generic sort method
    void sort() {
        if constexpr (!std::is_same_v<Container, std::set<typename Container::value_type>> &&
                     !std::is_same_v<Container, std::map<typename Container::key_type, typename Container::mapped_type>>) {
            std::sort(data.begin(), data.end());
        }
    }

    // Generic accumulate method
    template<typename T>
    T accumulate(const T& init) const {
        return std::accumulate(data.begin(), data.end(), init);
    }

    // Generic transform method
    template<typename UnaryOperation>
    void transform(UnaryOperation op) {
        std::transform(data.begin(), data.end(), data.begin(), op);
    }

    // Generic filter method
    template<typename Predicate>
    void filter(Predicate pred) {
        if constexpr (std::is_same_v<Container, std::vector<typename Container::value_type>> ||
                     std::is_same_v<Container, std::list<typename Container::value_type>> ||
                     std::is_same_v<Container, std::deque<typename Container::value_type>>) {
            data.erase(
                std::remove_if(data.begin(), data.end(), pred),
                data.end()
            );
        }
    }

    // Iterator methods
    iterator begin() { return data.begin(); }
    iterator end() { return data.end(); }
    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }

    // Size method
    size_t size() const { return data.size(); }
};

// Template function for printing container contents
template<typename Container>
void printContainer(const Container& container, const std::string& name) {
    std::cout << name << " contents: ";
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

// Template specialization for map printing
template<typename Key, typename Value>
void printContainer(const std::map<Key, Value>& container, const std::string& name) {
    std::cout << name << " contents: ";
    for (const auto& [key, value] : container) {
        std::cout << "{" << key << ": " << value << "} ";
    }
    std::cout << std::endl;
}

int main() {
    // Test with different container types
    ContainerWrapper<std::vector<int>> vecWrapper;
    ContainerWrapper<std::list<double>> listWrapper;
    ContainerWrapper<std::set<std::string>> setWrapper;
    ContainerWrapper<std::map<int, std::string>> mapWrapper;

    // Insert elements
    for (int i = 0; i < 5; ++i) {
        vecWrapper.insert(i);
        listWrapper.insert(static_cast<double>(i) + 0.5);
        setWrapper.insert("str" + std::to_string(i));
        mapWrapper.insert({i, "value" + std::to_string(i)});
    }

    // Print initial contents
    printContainer(vecWrapper, "Vector");
    printContainer(listWrapper, "List");
    printContainer(setWrapper, "Set");
    printContainer(mapWrapper, "Map");

    // Demonstrate sorting
    vecWrapper.sort();
    listWrapper.sort();
    std::cout << "\nAfter sorting:" << std::endl;
    printContainer(vecWrapper, "Vector");
    printContainer(listWrapper, "List");

    // Demonstrate transformation
    vecWrapper.transform([](int x) { return x * x; });
    listWrapper.transform([](double x) { return x * 2.0; });
    std::cout << "\nAfter transformation:" << std::endl;
    printContainer(vecWrapper, "Vector");
    printContainer(listWrapper, "List");

    // Demonstrate filtering
    vecWrapper.filter([](int x) { return x % 2 == 0; });
    listWrapper.filter([](double x) { return x > 5.0; });
    std::cout << "\nAfter filtering:" << std::endl;
    printContainer(vecWrapper, "Vector");
    printContainer(listWrapper, "List");

    // Demonstrate accumulation
    int sum = vecWrapper.accumulate(0);
    double product = listWrapper.accumulate(1.0);
    std::cout << "\nAccumulation results:" << std::endl;
    std::cout << "Vector sum: " << sum << std::endl;
    std::cout << "List product: " << product << std::endl;

    // Demonstrate finding elements
    auto vecIt = vecWrapper.find(4);
    auto listIt = listWrapper.find(7.0);
    auto setIt = setWrapper.find("str2");
    auto mapIt = mapWrapper.find(3);

    std::cout << "\nFind results:" << std::endl;
    std::cout << "Found in vector: " << (vecIt != vecWrapper.end() ? "Yes" : "No") << std::endl;
    std::cout << "Found in list: " << (listIt != listWrapper.end() ? "Yes" : "No") << std::endl;
    std::cout << "Found in set: " << (setIt != setWrapper.end() ? "Yes" : "No") << std::endl;
    std::cout << "Found in map: " << (mapIt != mapWrapper.end() ? "Yes" : "No") << std::endl;

    return 0;
} 