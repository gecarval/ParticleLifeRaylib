#include "Node2D.hpp"

Node2D::Node2D(const std::string &instanceName)
	: Object(instanceName), Node(instanceName), CanvasItem(instanceName) {
}

Node2D::Node2D(const Node2D &other)
	: Object(other), Node(other), CanvasItem(other) {
}

Node2D &Node2D::operator=(const Node2D &other) {
	if (this != &other) {
		CanvasItem::operator=(other);
		// Copy derived class members here
	}
	return *this;
}

Node2D::~Node2D() {
}

const std::string &Node2D::getClassName() const noexcept {
	static const std::string className = "Node2D";
	return className;
}
