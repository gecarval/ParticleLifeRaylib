#include "Sprite2D.hpp"
#include "../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"
#include <algorithm>

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

const Shape2D *Sprite2D::getShape() const noexcept {
	return _shape;
}

Shape2D *Sprite2D::getShape() noexcept {
	return _shape;
}

void Sprite2D::setShape(Shape2D *newShape) noexcept {
	if (_shape != nullptr) {
		delete _shape;
	}
	_shape = newShape;
}

const raylib::Texture2D &Sprite2D::getTexture() const noexcept {
	return _texture;
}

raylib::Texture2D &Sprite2D::getTexture() noexcept {
	return _texture;
}

void Sprite2D::setTexture(raylib::Texture2D &&newTexture) noexcept {
	_texture = std::move(newTexture);
}

raylib::Vector2 Sprite2D::getOffset() const noexcept {
	return _offset;
}

void Sprite2D::setOffset(const raylib::Vector2 &newOffset) noexcept {
	_offset = newOffset;
}

float Sprite2D::getScale() const noexcept {
	return _scale;
}

void Sprite2D::setScale(const float newScale) noexcept {
	_scale = newScale;
}

float Sprite2D::getRotation() const noexcept {
	return _rotation;
}

void Sprite2D::setRotation(const float newRotation) noexcept {
	_rotation = newRotation;
}

void Sprite2D::draw() const noexcept {
	if (_texture.id != 0) {
		_texture.Draw(_pos + _offset, _rot + _rotation, _scale, _color);
	} else if (_shape != nullptr) {
		if (_shape->getClassName() == "Circle2D") {
			CircleShape2D  *circle = dynamic_cast<CircleShape2D *>(_shape);
			raylib::Vector2 center = circle->getPosition();
			center += _pos + _offset;
			const float radius = circle->getRadius() * _scale;
			center.DrawCircle(radius, _color);
		} else if (_shape->getClassName() == "Rectangle2D") {
			RectangleShape2D *rectangle =
				dynamic_cast<RectangleShape2D *>(_shape);
			raylib::Rectangle rect = rectangle->getRect();
			rect.SetSize(rect.GetSize() * _scale);
			rect.Draw(_pos + _offset, _rot + _rotation, _color);
		}
	}
}

void Sprite2D::drawDebug() const noexcept {
	if (_shape != nullptr) {
		_shape->drawDebug();
	}
}

const std::string &Sprite2D::getClassName() const noexcept {
	static const std::string className("Sprite2D");
	return className;
}
