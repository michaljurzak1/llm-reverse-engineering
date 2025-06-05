#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <deque>

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
        if constexpr (std::is_same_v<Container, std::vector<value_type>>) {
            data.push_back(value);
        } else if constexpr (std::is_same_v<Container, std::list<value_type>>) {
            data.push_back(value);
        } else if constexpr (std::is_same_v<Container, std::set<value_type>>) {
            data.insert(value);
        } else if constexpr (std::is_same_v<Container, std::map<typename Container::key_type, typename Container::mapped_type>>) {
            if constexpr (std::is_same_v<T, std::pair<typename Container::key_type, typename Container::mapped_type>>) {
                data.insert(value);
            }
        }
    }

    // Generic find method
    template<typename T>
    iterator find(const T& value) {
        if constexpr (std::is_same_v<Container, std::vector<value_type>> || 
                     std::is_same_v<Container, std::list<value_type>>) {
            return std::find(data.begin(), data.end(), value);
        } else if constexpr (std::is_same_v<Container, std::set<value_type>>) {
            return data.find(value);
        } else if constexpr (std::is_same_v<Container, std::map<typename Container::key_type, typename Container::mapped_type>>) {
            return data.find(value);
        }
        return data.end(); // Default case
    }

    // Generic sort method
    void sort() {
        if constexpr (std::is_same_v<Container, std::vector<value_type>>) {
            std::sort(data.begin(), data.end());
        } else if constexpr (std::is_same_v<Container, std::list<value_type>>) {
            data.sort();
        }
    }

    // Generic accumulate method
    template<typename T>
    T accumulate(const T& init) const {
        if constexpr (std::is_same_v<Container, std::vector<value_type>> ||
                     std::is_same_v<Container, std::list<value_type>>) {
            return std::accumulate(data.begin(), data.end(), init);
        }
        return init;
    }

    // Generic transform method
    template<typename UnaryOperation>
    void transform(UnaryOperation op) {
        std::transform(data.begin(), data.end(), data.begin(), op);
    }

    // Generic filter method
    template<typename Predicate>
    void filter(Predicate pred) {
        if constexpr (std::is_same_v<Container, std::vector<value_type>> ||
                     std::is_same_v<Container, std::list<value_type>> ||
                     std::is_same_v<Container, std::deque<value_type>>) {
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
    std::cout << name << ": ";
    for (const auto& item : container) {
        if constexpr (std::is_same_v<typename Container::value_type, std::pair<const int, std::string>>) {
            std::cout << "(" << item.first << ", " << item.second << ") ";
        } else {
            std::cout << item << " ";
        }
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
        std::pair<const int, std::string> pair(i, "value" + std::to_string(i));
        mapWrapper.insert(pair);
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