#include "Node2D.hpp"

Node2D::Node2D(const std::string &instanceName)
	: Object(instanceName), Node(instanceName), CanvasItem(instanceName),
	  _pos(raylib::Vector2()), _vel(raylib::Vector2()), _acc(raylib::Vector2()),
	  _rot(0.0f) {
}

Node2D::Node2D(const Node2D &other)
	: Object(other), Node(other), CanvasItem(other), _pos(other._pos),
	  _vel(other._vel), _acc(other._acc), _rot(other._rot) {
}

Node2D &Node2D::operator=(const Node2D &other) {
	if (this != &other) {
		CanvasItem::operator=(other);
		_pos = other._pos;
		_vel = other._vel;
		_acc = other._acc;
		_rot = other._rot;
	}
	return *this;
}

Node2D::~Node2D() {
}

void Node2D::draw(const raylib::Window &window) const noexcept {
	// Implement drawing logic here
	(void)window;
}

void Node2D::draw(const raylib::Window	 &window,
				  const raylib::Camera2D &camera) const noexcept {
	// Implement drawing logic here
	(void)window;
	(void)camera;
}

void Node2D::drawDebug(const raylib::Window &window) const noexcept {
	const raylib::Rectangle screenSpace(0, 0, window.GetWidth(),
										window.GetHeight());
	if (_pos.CheckCollision(screenSpace)) {
		const raylib::Vector2 endPosX =
			_pos + raylib::Vector2(10.0f, 0).Rotate(_rot);
		const raylib::Vector2 endPosY =
			_pos + raylib::Vector2(0, 10.0f).Rotate(_rot);
		_pos.DrawLine(endPosX, raylib::Color::Red());
		_pos.DrawLine(endPosY, raylib::Color::Green());
		_pos.DrawCircle(0.5f, raylib::Color::Blue());
	}
}

void Node2D::drawDebug(const raylib::Window	  &window,
					   const raylib::Camera2D &camera) const noexcept {
	const raylib::Rectangle screenSpace(
		camera.GetTarget().x - (window.GetWidth() / 2.0f) / camera.GetZoom(),
		camera.GetTarget().y - (window.GetHeight() / 2.0f) / camera.GetZoom(),
		window.GetWidth() / camera.GetZoom(),
		window.GetHeight() / camera.GetZoom());
	if (_pos.CheckCollision(screenSpace)) {
		const raylib::Vector2 endPosX =
			_pos + raylib::Vector2(10.0f, 0).Rotate(_rot);
		const raylib::Vector2 endPosY =
			_pos + raylib::Vector2(0, 10.0f).Rotate(_rot);
		_pos.DrawLine(endPosX, raylib::Color::Red());
		_pos.DrawLine(endPosY, raylib::Color::Green());
		_pos.DrawCircle(0.5f, raylib::Color::Blue());
	}
}

const raylib::Vector2 &Node2D::getPos() const noexcept {
	return _pos;
}

raylib::Vector2 &Node2D::getPos() noexcept {
	return _pos;
}

void Node2D::setPos(const raylib::Vector2 &newPos) noexcept {
	_pos = newPos;
}

const raylib::Vector2 &Node2D::getVel() const noexcept {
	return _vel;
}

raylib::Vector2 &Node2D::getVel() noexcept {
	return _vel;
}

void Node2D::setVel(const raylib::Vector2 &newVel) noexcept {
	_vel = newVel;
}

const raylib::Vector2 &Node2D::getAcc() const noexcept {
	return _acc;
}

raylib::Vector2 &Node2D::getAcc() noexcept {
	return _acc;
}

void Node2D::setAcc(const raylib::Vector2 &newAcc) noexcept {
	_acc = newAcc;
}

float Node2D::getRotation() const noexcept {
	return _rot;
}

void Node2D::setRotation(const float newRot) noexcept {
	_rot = newRot;
}

const std::string &Node2D::getClassName() const noexcept {
	static const std::string className("Node2D");
	return className;
}
