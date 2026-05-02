#include "RectangleShape2D.hpp"
#include "../circle_shape2d/CircleShape2D.hpp"

RectangleShape2D::RectangleShape2D(const float x, const float y,
								   const float width, const float height)
	: Shape2D("RectangleShape2D"), _shape(x, y, width, height) {
}

RectangleShape2D::RectangleShape2D(const raylib::Vector2 &pos,
								   const raylib::Vector2 &size)
	: Shape2D("RectangleShape2D"), _shape(pos, size) {
}

RectangleShape2D::RectangleShape2D(const raylib::Rectangle &rect)
	: Shape2D("RectangleShape2D"), _shape(rect) {
}

RectangleShape2D::RectangleShape2D(const RectangleShape2D &other)
	: Shape2D(other) {
	_shape = other._shape;
}

RectangleShape2D &RectangleShape2D::operator=(const RectangleShape2D &other) {
	if (this != &other) {
		Shape2D::operator=(other);
		_shape = other._shape;
	}
	return *this;
}

RectangleShape2D::~RectangleShape2D() {
}

void RectangleShape2D::setRect(const raylib::Rectangle &rect) noexcept {
	_shape = rect;
}

raylib::Rectangle &RectangleShape2D::getRect() noexcept {
	return _shape;
}

const raylib::Rectangle &RectangleShape2D::getRect() const noexcept {
	return _shape;
}

void RectangleShape2D::drawDebug() const noexcept {
	static const raylib::Color debugColor =
		raylib::Color::DarkBlue().Fade(0.5f);
	_shape.Draw(debugColor);
	_shape.DrawLines(raylib::Color::DarkBlue());
}

bool RectangleShape2D::collides(
	const raylib::Vector2 &originPos, const Shape2D &other,
	const raylib::Vector2 &otherOriginPos) const noexcept {
	const raylib::Rectangle thisRect(_shape.x + originPos.x,
									 _shape.y + originPos.y, _shape.width,
									 _shape.height);
	if (other.getClassName() == "CircleShape2D") {
		const CircleShape2D &otherCircle =
			dynamic_cast<const CircleShape2D &>(other);
		const raylib::Vector2 otherShapePos =
			otherOriginPos + otherCircle.getPosition();
		return thisRect.CheckCollision(otherShapePos, otherCircle.getRadius());
	} else if (other.getClassName() == "RectangleShape2D") {
		const RectangleShape2D &otherRectShape =
			dynamic_cast<const RectangleShape2D &>(other);
		const raylib::Rectangle otherRect(
			otherRectShape.getRect().x + otherOriginPos.x,
			otherRectShape.getRect().y + otherOriginPos.y,
			otherRectShape.getRect().width, otherRectShape.getRect().height);
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
