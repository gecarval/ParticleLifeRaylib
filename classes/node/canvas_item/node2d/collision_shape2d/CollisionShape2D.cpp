#include "CollisionShape2D.hpp"
#include "../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"

CollisionShape2D::CollisionShape2D(const std::string &instanceName)
	: Node2D(instanceName), _shape(nullptr), _enabled(true) {
}

CollisionShape2D::CollisionShape2D(const raylib::Rectangle &rect,
								   const std::string	   &instanceName)
	: Node2D(instanceName), _shape(new RectangleShape2D(rect.GetSize())),
	  _enabled(true) {
	setPos(rect.GetPosition());
}

CollisionShape2D::CollisionShape2D(const raylib::Vector2 &point,
								   const float			  radius,
								   const std::string	 &instanceName)
	: Node2D(instanceName), _shape(new CircleShape2D(radius)), _enabled(true) {
	setPos(point);
}

CollisionShape2D::CollisionShape2D(const CollisionShape2D &other)
	: Node2D(other), _shape(other._shape), _enabled(other._enabled) {
}

CollisionShape2D &CollisionShape2D::operator=(const CollisionShape2D &other) {
	if (this != &other) {
		Node2D::operator=(other);
		_enabled = other._enabled;
	}
	return *this;
}

CollisionShape2D::~CollisionShape2D() {
	if (_shape != nullptr) {
		delete _shape;
		_shape = nullptr;
	}
}

void CollisionShape2D::drawDebug() const noexcept {
	if (_shape != nullptr) {
		if (_shape->getClassName() == "RectangleShape2D") {
			// Draw the rectangle centred on _pos and rotated by _rot,
			// matching how Sprite2D draws a RectangleShape2D.
			const RectangleShape2D *rect =
				dynamic_cast<const RectangleShape2D *>(_shape);
			static const raylib::Color debugColor =
				raylib::Color::DarkBlue().Fade(0.5f);
			const raylib::Vector2 size = rect->getSize();
			raylib::Rectangle	  debugRect(_pos.x, _pos.y, size.x, size.y);
			debugRect.Draw(size * 0.5f, _rot * RAD2DEG, debugColor);
		} else {
			_shape->drawDebug(_pos);
		}
	}
	Node2D::drawDebug();
}

void CollisionShape2D::setParentPos(const raylib::Vector2 &newPos) noexcept {
	if (_parent == nullptr) {
		return;
	}
	Node2D *_parentNode2D = dynamic_cast<Node2D *>(_parent);
	if (_parentNode2D == nullptr) {
		return;
	}
	_parentNode2D->setPos(newPos);
}

void CollisionShape2D::setParentRotation(const float newRotation) noexcept {
	if (_parent == nullptr) {
		return;
	}
	Node2D *_parentNode2D = dynamic_cast<Node2D *>(_parent);
	if (_parentNode2D == nullptr) {
		return;
	}
	_parentNode2D->setRotation(newRotation);
}

raylib::Vector2 CollisionShape2D::getParentPos() const noexcept {
	if (_parent == nullptr) {
		return raylib::Vector2();
	}
	const Node2D *_parentNode2D = dynamic_cast<const Node2D *>(_parent);
	if (_parentNode2D == nullptr) {
		return raylib::Vector2();
	}
	return _parentNode2D->getPos();
}

float CollisionShape2D::getParentRotation() const noexcept {
	if (_parent == nullptr) {
		return 0.0f;
	}
	const Node2D *_parentNode2D = dynamic_cast<const Node2D *>(_parent);
	if (_parentNode2D == nullptr) {
		return 0.0f;
	}
	return _parentNode2D->getRotation();
}

void CollisionShape2D::setShape(const float width,
								const float height) noexcept {
	const raylib::Vector2 newSize(width, height);
	if (_shape == nullptr) {
		_shape = new RectangleShape2D(newSize);
		return;
	}
	if (_shape->getClassName() == "RectangleShape2D") {
		RectangleShape2D *rectShape = dynamic_cast<RectangleShape2D *>(_shape);
		rectShape->setSize(newSize);
	} else {
		setShape(new RectangleShape2D(newSize));
	}
}

void CollisionShape2D::setShape(const raylib::Vector2 &size) noexcept {
	if (_shape == nullptr) {
		_shape = new RectangleShape2D(size);
		return;
	}
	if (_shape->getClassName() == "RectangleShape2D") {
		RectangleShape2D *rectShape = dynamic_cast<RectangleShape2D *>(_shape);
		rectShape->setSize(size);
	} else {
		setShape(new RectangleShape2D(size));
	}
}

void CollisionShape2D::setShape(const float radius) noexcept {
	if (_shape == nullptr) {
		_shape = new CircleShape2D(radius);
		return;
	}
	if (_shape->getClassName() == "CircleShape2D") {
		CircleShape2D *circleShape = dynamic_cast<CircleShape2D *>(_shape);
		circleShape->setRadius(radius);
	} else {
		setShape(new CircleShape2D(radius));
	}
}

const Shape2D *CollisionShape2D::getShape() const noexcept {
	return _shape;
}

Shape2D *CollisionShape2D::getShape() noexcept {
	return _shape;
}

void CollisionShape2D::setShape(Shape2D *newShape) noexcept {
	if (_shape != nullptr) {
		delete _shape;
	}
	_shape = newShape;
}

bool CollisionShape2D::isEnabled() const noexcept {
	return _enabled;
}

void CollisionShape2D::setEnabled(const bool newEnabled) noexcept {
	_enabled = newEnabled;
}

const std::string &CollisionShape2D::getClassName() const noexcept {
	static const std::string className("CollisionShape2D");
	return className;
}
