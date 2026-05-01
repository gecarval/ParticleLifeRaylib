#include "./Button.hpp"
#include "../../../../../include/raymath.h"

// Constructor
Button::Button(const float x, const float y, const float width,
			   const float height, const std::string &text)
	: bounds(x, y, width, height), text(text), fontSize(20), textColor(WHITE),
	  normalColor(100, 100, 100, 255), hoverColor(130, 130, 130, 255),
	  pressedColor(70, 70, 70, 255), borderColor(50, 50, 50, 255),
	  borderWidth(0.0f), roundness(0.0f), isHovered(false), isPressed(false),
	  wasPressed(false), enabled(true), onClick(nullptr) {
}

Button::Button(const float x, const float y, const raylib::Vector2 &size,
			   const std::string &text)
	: bounds(x, y, size.x, size.y), text(text), fontSize(20), textColor(WHITE),
	  normalColor(100, 100, 100, 255), hoverColor(130, 130, 130, 255),
	  pressedColor(70, 70, 70, 255), borderColor(50, 50, 50, 255),
	  borderWidth(0.0f), roundness(0.0f), isHovered(false), isPressed(false),
	  wasPressed(false), enabled(true), onClick(nullptr) {
}

Button::Button(const raylib::Vector2 &position, const float width,
			   const float height, const std::string &text)
	: bounds(position.x, position.y, width, height), text(text), fontSize(20),
	  textColor(WHITE), normalColor(100, 100, 100, 255),
	  hoverColor(130, 130, 130, 255), pressedColor(70, 70, 70, 255),
	  borderColor(50, 50, 50, 255), borderWidth(0.0f), roundness(0.0f),
	  isHovered(false), isPressed(false), wasPressed(false), enabled(true),
	  onClick(nullptr) {
}

Button::Button(const raylib::Vector2 &position, const raylib::Vector2 &size,
			   const std::string &text)
	: bounds(position.x, position.y, size.x, size.y), text(text), fontSize(20),
	  textColor(WHITE), normalColor(100, 100, 100, 255),
	  hoverColor(130, 130, 130, 255), pressedColor(70, 70, 70, 255),
	  borderColor(50, 50, 50, 255), borderWidth(0.0f), roundness(0.0f),
	  isHovered(false), isPressed(false), wasPressed(false), enabled(true),
	  onClick(nullptr) {
}

Button::Button(const raylib::Rectangle &bounds, const std::string &text)
	: bounds(bounds), text(text), fontSize(20), textColor(WHITE),
	  normalColor(100, 100, 100, 255), hoverColor(130, 130, 130, 255),
	  pressedColor(70, 70, 70, 255), borderColor(50, 50, 50, 255),
	  borderWidth(0.0f), roundness(0.0f), isHovered(false), isPressed(false),
	  wasPressed(false), enabled(true), onClick(nullptr) {
}

// Update method - checks for mouse interaction
void Button::update() {
	if (!this->enabled) {
		this->isHovered = false;
		this->isPressed = false;
		return;
	}

	this->checkInteraction();

	// Detect click event (button was pressed and now released)
	if (this->wasPressed && !this->isPressed && this->isHovered &&
		this->onClick) {
		this->onClick();
	}

	this->wasPressed = this->isPressed;
}

// Draw method - renders the button
void Button::draw() const {
	const raylib::Color &currentColor = this->getCurrentColor();

	// Draw button background
	if (this->roundness > 0.0f) {
		this->bounds.DrawRounded(this->roundness, 16, currentColor);
	} else {
		this->bounds.Draw(currentColor);
	}

	// Draw border if enabled
	if (this->borderWidth > 0.0f) {
		if (this->roundness > 0.0f) {
			this->bounds.DrawRoundedLines(this->roundness, 16,
										  this->borderColor);
		} else {
			this->bounds.DrawLines(this->borderColor, this->borderWidth);
		}
	}

	// Draw text centered
	raylib::Text		  text(this->text, this->fontSize, this->textColor,
							   ::GetFontDefault(), 1);
	const raylib::Vector2 textSize = text.MeasureEx();
	const raylib::Vector2 textPos(
		this->bounds.x + (this->bounds.width - textSize.x) / 2.0f,
		this->bounds.y + (this->bounds.height - textSize.y) / 2.0f);
	text.Draw(textPos);

	// Optional: Draw disabled overlay
	if (!this->enabled) {
		const raylib::Color &overlay = {0, 0, 0, 100};
		if (this->roundness > 0.0f) {
			this->bounds.DrawRounded(this->roundness, 16, overlay);
		} else {
			this->bounds.Draw(overlay);
		}
	}
}

// Setters
void Button::setText(const std::string &text) {
	this->text = text;
}

void Button::setPosition(const float x, const float y) {
	this->bounds.SetPosition(x, y);
}

void Button::setPosition(const raylib::Vector2 &position) {
	this->bounds.SetPosition(position);
}

void Button::setSize(const float width, const float height) {
	this->bounds.SetSize(width, height);
}

void Button::setSize(const raylib::Vector2 &size) {
	this->bounds.SetSize(size);
}

void Button::setBounds(const raylib::Rectangle &bounds) {
	this->bounds = bounds;
}

void Button::setColors(const raylib::Color &normal, const raylib::Color &hover,
					   const raylib::Color &pressed) {
	this->normalColor = normal;
	this->hoverColor = hover;
	this->pressedColor = pressed;
}

void Button::setTextColor(const raylib::Color &color) {
	this->textColor = color;
}

void Button::setFontSize(const unsigned int size) {
	this->fontSize = size;
}

void Button::setBorderWidth(const float width) {
	this->borderWidth = width;
}

void Button::setBorderColor(const raylib::Color &color) {
	this->borderColor = color;
}

void Button::setRoundness(const float roundness) {
	this->roundness = Clamp(roundness, 0.0f, 1.0f);
}

void Button::setOnClick(std::function<void()> callback) {
	this->onClick = callback;
}

// Getters
bool Button::isButtonPressed() const {
	return (this->isPressed);
}

bool Button::isButtonHovered() const {
	return (this->isHovered);
}

raylib::Rectangle &Button::getBounds() {
	return (this->bounds);
}

const raylib::Rectangle &Button::getBounds() const {
	return (this->bounds);
}

void Button::setEnabled(const bool enabled) {
	this->enabled = enabled;
}

bool Button::isEnabled() const {
	return (this->enabled);
}

// Private helper methods
raylib::Color Button::getCurrentColor() const {
	if (!this->enabled) {
		return (this->normalColor);
	}

	if (this->isPressed) {
		return (this->pressedColor);
	} else if (this->isHovered) {
		return (this->hoverColor);
	}
	return (this->normalColor);
}

void Button::checkInteraction() {
	const raylib::Vector2 &mousePos = raylib::Mouse::GetPosition();
	this->isHovered = this->bounds.CheckCollision(mousePos);

	if (this->isHovered && raylib::Mouse::IsButtonDown(MOUSE_LEFT_BUTTON)) {
		this->isPressed = true;
	} else if (raylib::Mouse::IsButtonReleased(MOUSE_LEFT_BUTTON)) {
		this->isPressed = false;
	}
}

const std::string &Button::getClassName() const noexcept {
	static const std::string className = "Button";
	return (className);
}
