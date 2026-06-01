#include "../../../../physics_server/PhysicsServer.hpp"
#include "CollisionObject2D.hpp"

CollisionObject2D::CollisionObject2D(const std::string &instanceName)
	: Node2D(instanceName), _collisionLayer(1), _collisionMask(1),
	  _enabled(true) {
	PhysicsServer::addCollisionObject(this);
}

CollisionObject2D::CollisionObject2D(const CollisionObject2D &other)
	: Node2D(other), _collisionLayer(other._collisionLayer),
	  _collisionMask(other._collisionMask), _enabled(other._enabled) {
	PhysicsServer::addCollisionObject(this);
}

CollisionObject2D &
CollisionObject2D::operator=(const CollisionObject2D &other) {
	if (this != &other) {
		Node2D::operator=(other);
		_collisionLayer = other._collisionLayer;
		_collisionMask = other._collisionMask;
		_enabled = other._enabled;
	}
	return *this;
}

CollisionObject2D::~CollisionObject2D() {
	PhysicsServer::removeCollisionObject(this);
}

int CollisionObject2D::getCollisionLayer() const noexcept {
	return _collisionLayer;
}

CollisionObject2D &CollisionObject2D::setCollisionLayer(int newLayer) noexcept {
	_collisionLayer = newLayer;
	return *this;
}

int CollisionObject2D::getCollisionMask() const noexcept {
	return _collisionMask;
}

CollisionObject2D &CollisionObject2D::setCollisionMask(int newMask) noexcept {
	_collisionMask = newMask;
	return *this;
}

bool CollisionObject2D::isEnabled() const noexcept {
	return _enabled;
}

CollisionObject2D &
CollisionObject2D::setEnabled(const bool newEnabled) noexcept {
	_enabled = newEnabled;
	return *this;
}

const std::string &CollisionObject2D::getClassName() const noexcept {
	static const std::string className("CollisionObject2D");
	return className;
}
