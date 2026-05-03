#include "CollisionObject2D.hpp"
#include "../../../../physics_server/PhysicsServer.hpp"

CollisionObject2D::CollisionObject2D(const std::string &instanceName)
	: Node2D(instanceName), _collisionShape(), _collisionLayer(1),
	  _collisionMask(1) {
	PhysicsServer::addCollisionObject(this);
}

CollisionObject2D::CollisionObject2D(const CollisionObject2D &other)
	: Node2D(other), _collisionShape(other._collisionShape),
	  _collisionLayer(other._collisionLayer),
	  _collisionMask(other._collisionMask) {
	PhysicsServer::addCollisionObject(this);
}

CollisionObject2D &
CollisionObject2D::operator=(const CollisionObject2D &other) {
	if (this != &other) {
		Node2D::operator=(other);
		_collisionShape = other._collisionShape;
		_collisionLayer = other._collisionLayer;
		_collisionMask = other._collisionMask;
	}
	return *this;
}

CollisionObject2D::~CollisionObject2D() {
	PhysicsServer::removeCollisionObject(this);
}

void CollisionObject2D::drawDebug() const noexcept {
	_collisionShape.drawDebug();
	Node2D::drawDebug();
}

const CollisionShape2D &CollisionObject2D::getCollisionShape() const noexcept {
	return _collisionShape;
}

CollisionShape2D &CollisionObject2D::getCollisionShape() noexcept {
	return _collisionShape;
}

void CollisionObject2D::setCollisionShape(
	const CollisionShape2D &newShape) noexcept {
	_collisionShape = newShape;
}

int CollisionObject2D::getCollisionLayer() const noexcept {
	return _collisionLayer;
}

void CollisionObject2D::setCollisionLayer(int newLayer) noexcept {
	_collisionLayer = newLayer;
}

int CollisionObject2D::getCollisionMask() const noexcept {
	return _collisionMask;
}

void CollisionObject2D::setCollisionMask(int newMask) noexcept {
	_collisionMask = newMask;
}

const std::string &CollisionObject2D::getClassName() const noexcept {
	static const std::string className("CollisionObject2D");
	return className;
}
