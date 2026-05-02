#include "Shape2D.hpp"

Shape2D::Shape2D(const std::string &instanceName) : Object(instanceName) {
}

Shape2D::Shape2D(const Shape2D &other) : Object(other) {
}

Shape2D &Shape2D::operator=(const Shape2D &other) {
	if (this != &other) {
		Object::operator=(other);
	}
	return *this;
}

Shape2D::~Shape2D() {
}

const std::string &Shape2D::getClassName() const noexcept {
	static const std::string className("Shape2D");
	return className;
}
