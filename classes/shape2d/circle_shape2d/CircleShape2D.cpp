#include "CircleShape2D.hpp"
#include "../rectangle_shape2d/RectangleShape2D.hpp"

CircleShape2D::CircleShape2D(const float x, const float y, const float radius)
	: Shape2D("CircleShape2D"), _pos(x, y), _radius(radius) {
}

CircleShape2D::CircleShape2D(const raylib::Vector2 &pos, const float radius)
	: Shape2D("CircleShape2D"), _pos(pos), _radius(radius) {
}

CircleShape2D::CircleShape2D(const CircleShape2D &other)
	: Shape2D(other), _pos(other._pos), _radius(other._radius) {
}

CircleShape2D &CircleShape2D::operator=(const CircleShape2D &other) {
	if (this != &other) {
		Shape2D::operator=(other);
		_pos = other._pos;
		_radius = other._radius;
	}
	return *this;
}

CircleShape2D::~CircleShape2D() {
}

void CircleShape2D::setPosition(const raylib::Vector2 &pos) noexcept {
	_pos = pos;
}

raylib::Vector2 CircleShape2D::getPosition() const noexcept {
	return _pos;
}

void CircleShape2D::setRadius(const float radius) noexcept {
	_radius = radius;
}

float CircleShape2D::getRadius() const noexcept {
	return _radius;
}

void CircleShape2D::drawDebug() const noexcept {
	static const raylib::Color debugColor =
		raylib::Color::DarkBlue().Fade(0.5f);
	_pos.DrawCircle(_radius, debugColor);
	_pos.DrawCircleLine(_radius, raylib::Color::DarkBlue());
}

bool CircleShape2D::collides(
	const raylib::Vector2 &originPos, const Shape2D &other,
	const raylib::Vector2 &otherOriginPos) const noexcept {
	const raylib::Vector2 thisCenter(_pos.x + originPos.x,
									 _pos.y + originPos.y);

	if (other.getClassName() == "CircleShape2D") {
		const CircleShape2D otherCircle =
			dynamic_cast<const CircleShape2D &>(other);
		const raylib::Vector2 otherShapePos(
			otherCircle._pos.x + otherOriginPos.x,
			otherCircle._pos.y + otherOriginPos.y);
		const float otherShapeRad = otherCircle._radius;
		return thisCenter.CheckCollisionCircle(_radius, otherShapePos,
											   otherShapeRad);
	} else if (other.getClassName() == "CircleShape2D") {
		const RectangleShape2D otherRectShape =
			dynamic_cast<const RectangleShape2D &>(other);
		const raylib::Rectangle otherShape = otherRectShape.getShape();
		const raylib::Rectangle otherRect(otherShape.x + otherOriginPos.x,
										  otherShape.y + otherOriginPos.y,
										  otherShape.width, otherShape.height);
		return thisCenter.CheckCollisionCircle(_radius, otherRect);
	}
	return (false);
}

bool CircleShape2D::collidesOnMotion(
	const raylib::Vector2 &originPos, const raylib::Vector2 &velocity,
	const Shape2D &other, const raylib::Vector2 &otherOriginPos,
	const raylib::Vector2 &otherVelocity) const noexcept {
	const float			  deltaTime = raylib::Window::GetFrameTime();
	const raylib::Vector2 midPos = originPos + velocity * deltaTime * 0.5f;
	const raylib::Vector2 otherMidPos =
		otherOriginPos + otherVelocity * deltaTime * 0.5f;
	return collides(midPos, other, otherMidPos);
}

const std::string &CircleShape2D::getClassName() const noexcept {
	static const std::string className("CircleShape2D");
	return className;
}
