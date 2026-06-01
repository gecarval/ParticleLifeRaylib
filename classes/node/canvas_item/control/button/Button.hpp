#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../../../../../include/raylib-cpp.hpp"
#include "../Control.hpp"
#include <functional>
#include <string>

class Button : public Control {
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
	Button		 &checkInteraction();

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
	virtual void update() noexcept override;
	virtual void draw() const noexcept override;

	// Setters for customization
	Button &setText(const std::string &text);
	Button &setPosition(const float x, const float y);
	Button &setPosition(const raylib::Vector2 &position);
	Button &setSize(float width, float height);
	Button &setSize(const raylib::Vector2 &size);
	Button &setBounds(const raylib::Rectangle &size);
	Button &setColors(const raylib::Color &normal, const raylib::Color &hover,
					  const raylib::Color &pressed);
	Button &setTextColor(const raylib::Color &color);
	Button &setFontSize(const unsigned int size);
	Button &setBorderWidth(const float width);
	Button &setBorderColor(const raylib::Color &color);
	Button &setRoundness(const float roundness);
	Button &setOnClick(std::function<void()> callback);

	// Getters
	bool					 isButtonPressed() const;
	bool					 isButtonHovered() const;
	raylib::Rectangle		&getBounds();
	const raylib::Rectangle &getBounds() const;

	// State management
	Button &setEnabled(const bool enabled);
	bool	isEnabled() const;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // BUTTON_HPP
