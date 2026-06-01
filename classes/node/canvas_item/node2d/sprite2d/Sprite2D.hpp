#ifndef SPRITE2D_HPP
#define SPRITE2D_HPP

#include "../../../../shape2d/Shape2D.hpp"
#include "../Node2D.hpp"

class Sprite2D : public Node2D {
  protected:
	Shape2D			 *_shape;
	raylib::Texture2D _texture;
	raylib::Color	  _color;
	raylib::Vector2	  _offset;
	float			  _scale;
	float			  _rotation;

	Sprite2D(const Sprite2D &other) = delete;
	Sprite2D &operator=(const Sprite2D &other) = delete;

  public:
	Sprite2D(const std::string &instanceName = "");
	Sprite2D(Sprite2D &&other);
	Sprite2D &operator=(Sprite2D &&other);
	virtual ~Sprite2D();

	// utility functions
	Sprite2D &setShape(const float width, const float height) noexcept;
	Sprite2D &setShape(const raylib::Vector2 &size) noexcept;
	Sprite2D &setShape(const float radius) noexcept;

	// getters and setters
	const Shape2D			*getShape() const noexcept;
	Shape2D					*getShape() noexcept;
	Sprite2D				&setShape(Shape2D *newShape) noexcept;
	const raylib::Texture2D &getTexture() const noexcept;
	raylib::Texture2D		&getTexture() noexcept;
	Sprite2D &setTexture(raylib::Texture2D &&newTexture) noexcept;

	raylib::Color	getColor() const noexcept;
	Sprite2D	   &setColor(const raylib::Color &newColor) noexcept;
	raylib::Vector2 getOffset() const noexcept;
	Sprite2D	   &setOffset(const raylib::Vector2 &newOffset) noexcept;
	float			getScale() const noexcept;
	Sprite2D	   &setScale(const float newScale) noexcept;
	float			getRotation() const noexcept;
	Sprite2D	   &setRotation(const float newRotation) noexcept;

	// draw calls
	virtual void draw() const noexcept override;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // SPRITE2D_HPP
