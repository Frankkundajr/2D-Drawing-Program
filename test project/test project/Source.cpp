#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <memory>

// Base class for all shapes
class Shape {
public:
    virtual void draw(sf::RenderTarget& target) = 0;
    virtual ~Shape() = default;
};

// Derived class for Line
class Line : public Shape {
private:
    sf::VertexArray line;
public:
    Line(const sf::Vector2f& start, const sf::Vector2f& end, const sf::Color& color)
        : line(sf::Lines, 2) {
        line[0].position = start;
        line[1].position = end;
        line[0].color = color;
        line[1].color = color;
    }

    void draw(sf::RenderTarget& target) override {
        target.draw(line);
    }
};

// Derived class for Rectangle
class Rectangle : public Shape {
private:
    sf::RectangleShape rectangle;
public:
    Rectangle(const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color) {
        rectangle.setPosition(position);
        rectangle.setSize(size);
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineThickness(2);
        rectangle.setOutlineColor(color);
    }

    void draw(sf::RenderTarget& target) override {
        target.draw(rectangle);
    }
};

// Derived class for Circle
class Circle : public Shape {
private:
    sf::CircleShape circle;
public:
    Circle(const sf::Vector2f& position, float radius, const sf::Color& color) {
        circle.setPosition(position);
        circle.setRadius(radius);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(2);
        circle.setOutlineColor(color);
    }

    void draw(sf::RenderTarget& target) override {
        target.draw(circle);
    }
};

// GUI Button class for user-friendly interface
class Button {
private:
    sf::RectangleShape buttonShape;
    sf::Text buttonText;
    sf::Font font;

public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& text) {
        buttonShape.setPosition(position);
        buttonShape.setSize(size);
        buttonShape.setFillColor(sf::Color(100, 100, 100)); // Dark grey background

        if (!font.loadFromFile("arial.ttf")) {
            // Handle error
        }

        buttonText.setFont(font);
        buttonText.setString(text);
        buttonText.setCharacterSize(18);
        buttonText.setFillColor(sf::Color::White);
        buttonText.setPosition(position.x + 10, position.y + 10);
    }

    void draw(sf::RenderTarget& target) {
        target.draw(buttonShape);
        target.draw(buttonText);
    }

    bool isClicked(const sf::Vector2i& mousePos) {
        return buttonShape.getGlobalBounds().contains(sf::Vector2f(mousePos));
    }
};

// Main Application Class
class GraphicsApp {
private:
    sf::RenderWindow window;
    std::vector<std::shared_ptr<Shape>> shapes;
    sf::Color currentColor = sf::Color::White;
    sf::Font font;
    sf::Text instructions;

    enum class ShapeType { None, Line, Rectangle, Circle } currentShapeType;

    bool isDrawing = false; // True if we are in the drawing phase
    sf::Vector2f startPos;

    Button lineButton, rectButton, circleButton, clearButton, saveButton, undoButton;
    std::vector<std::shared_ptr<Shape>> undoStack; // For undo functionality

public:
    GraphicsApp()
        : window(sf::VideoMode(800, 600), "2D Graphics Drawing App"),
        currentShapeType(ShapeType::None),
        lineButton({ 10, 10 }, { 100, 50 }, "Line"),
        rectButton({ 120, 10 }, { 100, 50 }, "Rectangle"),
        circleButton({ 230, 10 }, { 100, 50 }, "Circle"),
        clearButton({ 340, 10 }, { 100, 50 }, "Clear"),
        saveButton({ 450, 10 }, { 100, 50 }, "Save"),
        undoButton({ 560, 10 }, { 100, 50 }, "Undo") {

        if (!font.loadFromFile("arial.ttf")) {
            // Handle font loading error
        }

        instructions.setFont(font);
        instructions.setString("Select a shape to draw. Press 'C' to clear.");
        instructions.setCharacterSize(20);
        instructions.setFillColor(sf::Color::White);
        instructions.setPosition(10, 70);
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            drawShapes();
        }
    }

private:
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (!isDrawing) {
                    // Start drawing on first click after selecting shape
                    if (lineButton.isClicked(mousePos)) {
                        currentShapeType = ShapeType::Line;
                    }
                    else if (rectButton.isClicked(mousePos)) {
                        currentShapeType = ShapeType::Rectangle;
                    }
                    else if (circleButton.isClicked(mousePos)) {
                        currentShapeType = ShapeType::Circle;
                    }
                    else if (clearButton.isClicked(mousePos)) {
                        shapes.clear();
                        undoStack.clear();
                    }
                    else if (undoButton.isClicked(mousePos)) {
                        if (!shapes.empty()) {
                            undoStack.push_back(shapes.back());
                            shapes.pop_back();
                        }
                    }
                    else if (saveButton.isClicked(mousePos)) {
                        saveDrawing();
                    }
                    else if (currentShapeType != ShapeType::None) {
                        // If a shape type is selected, begin drawing
                        startPos = sf::Vector2f(mousePos);
                        isDrawing = true; // Mark the drawing phase as started
                    }
                }
                else {
                    // Complete the drawing on second click
                    sf::Vector2f endPos(event.mouseButton.x, event.mouseButton.y);
                    if (currentShapeType == ShapeType::Line) {
                        shapes.push_back(std::make_shared<Line>(startPos, endPos, currentColor));
                    }
                    else if (currentShapeType == ShapeType::Rectangle) {
                        sf::Vector2f size = endPos - startPos;
                        shapes.push_back(std::make_shared<Rectangle>(startPos, size, currentColor));
                    }
                    else if (currentShapeType == ShapeType::Circle) {
                        float radius = std::sqrt(std::pow(endPos.x - startPos.x, 2) + std::pow(endPos.y - startPos.y, 2));
                        shapes.push_back(std::make_shared<Circle>(startPos, radius, currentColor));
                    }
                    currentShapeType = ShapeType::None;
                    isDrawing = false; // Mark the drawing as completed
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
                shapes.clear();
                undoStack.clear();
            }
        }
    }

    void drawShapes() {
        window.clear(sf::Color::Black);

        // Draw GUI buttons
        lineButton.draw(window);
        rectButton.draw(window);
        circleButton.draw(window);
        clearButton.draw(window);
        saveButton.draw(window);
        undoButton.draw(window);

        // Draw instructions
        window.draw(instructions);

        // Draw shapes
        for (const auto& shape : shapes) {
            shape->draw(window);
        }

        window.display();
    }

    // Save the current drawing to a PNG file
    void saveDrawing() {
        sf::RenderTexture texture;
        texture.create(window.getSize().x, window.getSize().y);

        // Clear the texture
        texture.clear(sf::Color::Black);

        // Draw all the shapes to the texture
        for (const auto& shape : shapes) {
            shape->draw(texture);
        }

        // Save the texture to a PNG file
        texture.display();
        texture.getTexture().copyToImage().saveToFile("drawing.png");
    }
};

int main() {
    GraphicsApp app;
    app.run();
    return 0;
}
