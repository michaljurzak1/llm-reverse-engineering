#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Base class for shapes
class Shape {
protected:
    std::string name;
    double x, y;

public:
    Shape(const std::string& name, double x, double y)
        : name(name), x(x), y(y) {
        std::cout << "Shape constructor: " << name << std::endl;
    }

    virtual ~Shape() {
        std::cout << "Shape destructor: " << name << std::endl;
    }

    // Pure virtual functions
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual void draw() const = 0;

    // Non-virtual function
    void move(double newX, double newY) {
        x = newX;
        y = newY;
        std::cout << "Moved " << name << " to (" << x << ", " << y << ")" << std::endl;
    }

    // Virtual function with default implementation
    virtual std::string getInfo() const {
        return "Shape: " + name + " at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

// Derived class: Circle
class Circle : public Shape {
private:
    double radius;

public:
    Circle(const std::string& name, double x, double y, double radius)
        : Shape(name, x, y), radius(radius) {
        std::cout << "Circle constructor: " << name << std::endl;
    }

    ~Circle() override {
        std::cout << "Circle destructor: " << name << std::endl;
    }

    double area() const override {
        return 3.14159 * radius * radius;
    }

    double perimeter() const override {
        return 2 * 3.14159 * radius;
    }

    void draw() const override {
        std::cout << "Drawing circle: " << name << " with radius " << radius << std::endl;
    }

    std::string getInfo() const override {
        return Shape::getInfo() + ", radius: " + std::to_string(radius);
    }
};

// Derived class: Rectangle
class Rectangle : public Shape {
private:
    double width, height;

public:
    Rectangle(const std::string& name, double x, double y, double width, double height)
        : Shape(name, x, y), width(width), height(height) {
        std::cout << "Rectangle constructor: " << name << std::endl;
    }

    ~Rectangle() override {
        std::cout << "Rectangle destructor: " << name << std::endl;
    }

    double area() const override {
        return width * height;
    }

    double perimeter() const override {
        return 2 * (width + height);
    }

    void draw() const override {
        std::cout << "Drawing rectangle: " << name 
                  << " with width " << width 
                  << " and height " << height << std::endl;
    }

    std::string getInfo() const override {
        return Shape::getInfo() + ", width: " + std::to_string(width) 
               + ", height: " + std::to_string(height);
    }
};

// Derived class: Triangle
class Triangle : public Shape {
private:
    double base, height;

public:
    Triangle(const std::string& name, double x, double y, double base, double height)
        : Shape(name, x, y), base(base), height(height) {
        std::cout << "Triangle constructor: " << name << std::endl;
    }

    ~Triangle() override {
        std::cout << "Triangle destructor: " << name << std::endl;
    }

    double area() const override {
        return 0.5 * base * height;
    }

    double perimeter() const override {
        // Simplified perimeter calculation for demonstration
        return base + 2 * std::sqrt(height * height + (base/2) * (base/2));
    }

    void draw() const override {
        std::cout << "Drawing triangle: " << name 
                  << " with base " << base 
                  << " and height " << height << std::endl;
    }

    std::string getInfo() const override {
        return Shape::getInfo() + ", base: " + std::to_string(base) 
               + ", height: " + std::to_string(height);
    }
};

// Function to demonstrate polymorphic behavior
void processShape(const Shape& shape) {
    std::cout << "\nProcessing shape:" << std::endl;
    std::cout << shape.getInfo() << std::endl;
    std::cout << "Area: " << shape.area() << std::endl;
    std::cout << "Perimeter: " << shape.perimeter() << std::endl;
    shape.draw();
}

int main() {
    // Create a vector of unique pointers to shapes
    std::vector<std::unique_ptr<Shape>> shapes;

    // Add different shapes to the vector
    shapes.push_back(std::make_unique<Circle>("Circle1", 0, 0, 5));
    shapes.push_back(std::make_unique<Rectangle>("Rectangle1", 10, 10, 4, 6));
    shapes.push_back(std::make_unique<Triangle>("Triangle1", 20, 20, 8, 6));

    // Demonstrate polymorphic behavior
    std::cout << "\nDemonstrating polymorphic behavior:" << std::endl;
    for (const auto& shape : shapes) {
        processShape(*shape);
    }

    // Demonstrate moving shapes
    std::cout << "\nDemonstrating shape movement:" << std::endl;
    for (auto& shape : shapes) {
        shape->move(shape->area(), shape->perimeter());
    }

    // Demonstrate virtual destructor behavior
    std::cout << "\nDemonstrating virtual destructor behavior:" << std::endl;
    shapes.clear(); // This will call the appropriate destructors

    return 0;
} 