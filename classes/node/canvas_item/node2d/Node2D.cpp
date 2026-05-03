#include "Node2D.hpp"

Node2D::Node2D(const std::string &instanceName)
	: CanvasItem(instanceName), _pos(raylib::Vector2()), _rot(0.0f) {
}

Node2D::Node2D(const Node2D &other)
	: CanvasItem(other), _pos(other._pos), _rot(other._rot) {
}

Node2D &Node2D::operator=(const Node2D &other) {
	if (this != &other) {
		CanvasItem::operator=(other);
		_pos = other._pos;
		_rot = other._rot;
	}
	return *this;
}

Node2D::~Node2D() {
}

void Node2D::draw() const noexcept {
}

void Node2D::drawDebug() const noexcept {
	const raylib::Vector2 endPosX =
		_pos + raylib::Vector2(10.0f, 0).Rotate(_rot);
	const raylib::Vector2 endPosY =
		_pos + raylib::Vector2(0, 10.0f).Rotate(_rot);
	_pos.DrawLine(endPosX, raylib::Color::Red());
	_pos.DrawLine(endPosY, raylib::Color::Green());
	_pos.DrawCircle(0.5f, raylib::Color::Blue());
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

float Node2D::getRotation() const noexcept {
	return _rot;
}

void Node2D::addRotation(const float newRot) noexcept {
	_rot += newRot;
	for (Node *child : _children) {
		if (Node2D *childNode2D = dynamic_cast<Node2D *>(child)) {
			childNode2D->addRotation(newRot);
		}
	}
}

void Node2D::resetRotation() noexcept {
	const float deltaRot = _rot;
	_rot = 0.0f;
	for (Node *child : _children) {
		if (Node2D *childNode2D = dynamic_cast<Node2D *>(child)) {
			childNode2D->addRotation(-deltaRot);
		}
	}
}

const std::string &Node2D::getClassName() const noexcept {
	static const std::string className("Node2D");
	return className;
}
