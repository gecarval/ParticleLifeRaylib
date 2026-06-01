#include "../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"
#include "Sprite2D.hpp"

Sprite2D::Sprite2D(const std::string &instanceName)
	: Node2D(instanceName), _shape(nullptr), _texture(),
	  _offset(raylib::Vector2::Zero()), _scale(1.0f), _rotation(0.0f) {
}

Sprite2D::Sprite2D(Sprite2D &&other)
	: Node2D(other), _shape(other._shape), _texture(std::move(other._texture)),
	  _offset(other._offset), _scale(other._scale), _rotation(other._rotation) {
	other._shape = nullptr;
}

Sprite2D &Sprite2D::operator=(Sprite2D &&other) {
	if (this != &other) {
		Node2D::operator=(other);
		if (_shape != nullptr) {
			delete _shape;
		}
		_shape = other._shape;
		other._shape = nullptr;
		_texture = std::move(other._texture);
		_offset = other._offset;
		_scale = other._scale;
		_rotation = other._rotation;
	}
	return *this;
}

Sprite2D::~Sprite2D() {
	if (_shape != nullptr) {
		delete _shape;
	}
}

Sprite2D &Sprite2D::setShape(const float width, const float height) noexcept {
	const raylib::Vector2 newSize(width, height);
	if (_shape == nullptr) {
		_shape = new RectangleShape2D(newSize);
		return *this;
	}
	if (_shape->getClassName() == "RectangleShape2D") {
		RectangleShape2D *rectShape = dynamic_cast<RectangleShape2D *>(_shape);
		rectShape->setSize(newSize);
	} else {
		setShape(new RectangleShape2D(newSize));
	}
	return *this;
}

Sprite2D &Sprite2D::setShape(const raylib::Vector2 &size) noexcept {
	if (_shape == nullptr) {
		_shape = new RectangleShape2D(size);
		return *this;
	}
	if (_shape->getClassName() == "RectangleShape2D") {
		RectangleShape2D *rectShape = dynamic_cast<RectangleShape2D *>(_shape);
		rectShape->setSize(size);
	} else {
		setShape(new RectangleShape2D(size));
	}
	return *this;
}

Sprite2D &Sprite2D::setShape(const float radius) noexcept {
	if (_shape == nullptr) {
		_shape = new CircleShape2D(radius);
		return *this;
	}
	if (_shape->getClassName() == "CircleShape2D") {
		CircleShape2D *circleShape = dynamic_cast<CircleShape2D *>(_shape);
		circleShape->setRadius(radius);
	} else {
		setShape(new CircleShape2D(radius));
	}
	return *this;
}

const Shape2D *Sprite2D::getShape() const noexcept {
	return _shape;
}

Shape2D *Sprite2D::getShape() noexcept {
	return _shape;
}

Sprite2D &Sprite2D::setShape(Shape2D *newShape) noexcept {
	if (_shape != nullptr) {
		delete _shape;
	}
	_shape = newShape;
	return *this;
}

const raylib::Texture2D &Sprite2D::getTexture() const noexcept {
	return _texture;
}

raylib::Texture2D &Sprite2D::getTexture() noexcept {
	return _texture;
}

Sprite2D &Sprite2D::setTexture(raylib::Texture2D &&newTexture) noexcept {
	_texture = std::move(newTexture);
	return *this;
}

raylib::Color Sprite2D::getColor() const noexcept {
	return _color;
}

Sprite2D &Sprite2D::setColor(const raylib::Color &newColor) noexcept {
	_color = newColor;
	return *this;
}

raylib::Vector2 Sprite2D::getOffset() const noexcept {
	return _offset;
}

Sprite2D &Sprite2D::setOffset(const raylib::Vector2 &newOffset) noexcept {
	_offset = newOffset;
	return *this;
}

float Sprite2D::getScale() const noexcept {
	return _scale;
}

Sprite2D &Sprite2D::setScale(const float newScale) noexcept {
	_scale = newScale;
	return *this;
}

float Sprite2D::getRotation() const noexcept {
	return _rotation;
}

Sprite2D &Sprite2D::setRotation(const float newRotation) noexcept {
	_rotation = newRotation;
	return *this;
}

void Sprite2D::draw() const noexcept {
	if (_texture.id != 0) {
		_texture.Draw(_pos + _offset, (_rot + _rotation) * RAD2DEG, _scale,
					  _color);
	} else if (_shape != nullptr) {
		if (_shape->getClassName() == "CircleShape2D") {
			CircleShape2D  *circle = dynamic_cast<CircleShape2D *>(_shape);
			raylib::Vector2 center = _pos + _offset;
			const float		radius = circle->getRadius() * _scale;
			center.DrawCircle(radius, _color);
		} else if (_shape->getClassName() == "RectangleShape2D") {
			RectangleShape2D *rectangle =
				dynamic_cast<RectangleShape2D *>(_shape);
			const raylib::Vector2 size = rectangle->getSize() * _scale;
			const raylib::Vector2 centre = _pos + _offset;
			raylib::Rectangle	  rect(centre.x, centre.y, size.x, size.y);
			rect.Draw(size * 0.5f, (_rot + _rotation) * RAD2DEG, _color);
		}
	}
}

const std::string &Sprite2D::getClassName() const noexcept {
	static const std::string className("Sprite2D");
	return className;
}
