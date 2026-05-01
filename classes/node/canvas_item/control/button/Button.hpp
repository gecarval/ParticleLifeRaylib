#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../../../../../include/raylib-cpp.hpp"
#include <functional>
#include <string>

class Button {
  private:
	// Position and size
	raylib::Rectangle bounds;

	// Text properties
	std::string	  text;
	unsigned int  fontSize;
	raylib::Color textColor;

	// Visual properties
	raylib::Color normalColor;
	raylib::Color hoverColor;
	raylib::Color pressedColor;
	raylib::Color borderColor;
	float		  borderWidth;
	float		  roundness;

	// State
	bool isHovered;
	bool isPressed;
	bool wasPressed;
	bool enabled;

	// Callback
	std::function<void()> onClick;

	// Helper methods
	raylib::Color getCurrentColor() const;
	void		  checkInteraction();

  public:
	// Constructor
	Button(const float x, const float y, const float width, const float height,
		   const std::string &text);
	Button(const float x, const float y, const raylib::Vector2 &size,
		   const std::string &text);
	Button(const raylib::Vector2 &position, const float width,
		   const float height, const std::string &text);
	Button(const raylib::Vector2 &position, const raylib::Vector2 &size,
		   const std::string &text);
	Button(const raylib::Rectangle &button, const std::string &text);

	// Update and draw methods
	void update();
	void draw() const;

	// Setters for customization
	void setText(const std::string &text);
	void setPosition(const float x, const float y);
	void setPosition(const raylib::Vector2 &position);
	void setSize(float width, float height);
	void setSize(const raylib::Vector2 &size);
	void setBounds(const raylib::Rectangle &size);
	void setColors(const raylib::Color &normal, const raylib::Color &hover,
				   const raylib::Color &pressed);
	void setTextColor(const raylib::Color &color);
	void setFontSize(const unsigned int size);
	void setBorderWidth(const float width);
	void setBorderColor(const raylib::Color &color);
	void setRoundness(const float roundness);
	void setOnClick(std::function<void()> callback);

	// Getters
	bool					 isButtonPressed() const;
	bool					 isButtonHovered() const;
	raylib::Rectangle		&getBounds();
	const raylib::Rectangle &getBounds() const;

	// State management
	void setEnabled(const bool enabled);
	bool isEnabled() const;

	virtual const std::string &getClassName() const noexcept;
};

#endif // BUTTON_HPP
