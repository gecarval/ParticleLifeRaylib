#include "Area2D.hpp"

Area2D::Area2D() {
}

Area2D::Area2D(const Area2D &other) : CollisionObject2D(other) {
	if (this != &other) {
		*this = other;
	}
}

Area2D &Area2D::operator=(const Area2D &other) {
	if (this != &other) {
		CollisionObject2D::operator=(other);
		// Copy derived class members here
	}
	return *this;
}

Area2D::~Area2D() {
}
