#include "CircleShape2D.hpp"
#include "../rectangle_shape2d/RectangleShape2D.hpp"

CircleShape2D::CircleShape2D(const float radius)
	: Shape2D("CircleShape2D"), _radius(radius) {
}

CircleShape2D::CircleShape2D(const CircleShape2D &other)
	: Shape2D(other), _radius(other._radius) {
}

CircleShape2D &CircleShape2D::operator=(const CircleShape2D &other) {
	if (this != &other) {
		Shape2D::operator=(other);
		_radius = other._radius;
	}
	return *this;
}

CircleShape2D::~CircleShape2D() {
}

void CircleShape2D::setRadius(const float radius) noexcept {
	_radius = radius;
}

float CircleShape2D::getRadius() const noexcept {
	return _radius;
}

void CircleShape2D::drawDebug(const raylib::Vector2 &pos) const noexcept {
	static const raylib::Color debugColor =
		raylib::Color::DarkBlue().Fade(0.5f);
	pos.DrawCircle(_radius, debugColor);
	pos.DrawCircleLine(_radius, raylib::Color::DarkBlue());
}

bool CircleShape2D::collides(
	const raylib::Vector2 &originPos, const Shape2D &other,
	const raylib::Vector2 &otherOriginPos) const noexcept {
	if (other.getClassName() == "CircleShape2D") {
		const CircleShape2D &otherCircle =
			dynamic_cast<const CircleShape2D &>(other);
		return originPos.CheckCollisionCircle(_radius, otherOriginPos,
											  otherCircle._radius);
	} else if (other.getClassName() == "RectangleShape2D") {
		const RectangleShape2D &otherRectShape =
			dynamic_cast<const RectangleShape2D &>(other);
		const raylib::Vector2  &otherSize = otherRectShape.getSize();
		const raylib::Rectangle otherRect(otherOriginPos.x, otherOriginPos.y,
										  otherSize.x, otherSize.y);
		return originPos.CheckCollisionCircle(_radius, otherRect);
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
