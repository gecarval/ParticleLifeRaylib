#include "RectangleShape2D.hpp"
#include "../circle_shape2d/CircleShape2D.hpp"

RectangleShape2D::RectangleShape2D(const float width, const float height)
	: Shape2D("RectangleShape2D"), _size(width, height) {
}

RectangleShape2D::RectangleShape2D(const raylib::Vector2 &size)
	: Shape2D("RectangleShape2D"), _size(size) {
}

RectangleShape2D::RectangleShape2D(const RectangleShape2D &other)
	: Shape2D(other), _size(other._size) {
}

RectangleShape2D &RectangleShape2D::operator=(const RectangleShape2D &other) {
	if (this != &other) {
		Shape2D::operator=(other);
		_size = other._size;
	}
	return *this;
}

RectangleShape2D::~RectangleShape2D() {
}

raylib::Vector2 RectangleShape2D::getSize() const noexcept {
	return _size;
}

void RectangleShape2D::setSize(const raylib::Vector2 &size) noexcept {
	_size = size;
}

void RectangleShape2D::drawDebug(const raylib::Vector2 &pos) const noexcept {
	static const raylib::Color debugColor =
		raylib::Color::DarkBlue().Fade(0.5f);
	const raylib::Rectangle rect(pos, _size);
	rect.Draw(debugColor);
	rect.DrawLines(raylib::Color::DarkBlue());
}

bool RectangleShape2D::collides(
	const raylib::Vector2 &originPos, const Shape2D &other,
	const raylib::Vector2 &otherOriginPos) const noexcept {
	const raylib::Rectangle thisRect(originPos.x, originPos.y, _size.x,
									 _size.y);
	if (other.getClassName() == "CircleShape2D") {
		const CircleShape2D &otherCircle =
			dynamic_cast<const CircleShape2D &>(other);
		return thisRect.CheckCollision(otherOriginPos, otherCircle.getRadius());
	} else if (other.getClassName() == "RectangleShape2D") {
		const RectangleShape2D &otherRectShape =
			dynamic_cast<const RectangleShape2D &>(other);
		const raylib::Rectangle otherRect(otherOriginPos,
										  otherRectShape.getSize());
		return thisRect.CheckCollision(otherRect);
	}
	return (false);
}

bool RectangleShape2D::collidesOnMotion(
	const raylib::Vector2 &originPos, const raylib::Vector2 &thisVel,
	const Shape2D &other, const raylib::Vector2 &otherOriginPos,
	const raylib::Vector2 &otherVel) const noexcept {
	const float			  deltaTime = raylib::Window::GetFrameTime();
	const raylib::Vector2 midPos = originPos + thisVel * deltaTime * 0.5f;
	const raylib::Vector2 otherMidPos =
		otherOriginPos + otherVel * deltaTime * 0.5f;
	return collides(midPos, other, otherMidPos);
}

const std::string &RectangleShape2D::getClassName() const noexcept {
	static const std::string className("RectangleShape2D");
	return className;
}
