#include "PhysicsBody2D.hpp"
#include "../../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"

PhysicsBody2D::PhysicsBody2D(const std::string &instanceName)
	: CollisionObject2D(instanceName), _linearVel(0, 0), _angularVel(0, 0),
	  _linearAcc(0, 0), _angularAcc(0, 0), _mass(1.0f), _friction(0.1f),
	  _restitution(0.5f), _is_static(false), _lock_rotation(false) {
}

PhysicsBody2D::PhysicsBody2D(const PhysicsBody2D &other)
	: CollisionObject2D(other), _linearVel(other._linearVel),
	  _angularVel(other._angularVel), _linearAcc(other._linearAcc),
	  _angularAcc(other._angularAcc), _mass(other._mass),
	  _friction(other._friction), _restitution(other._restitution),
	  _is_static(other._is_static), _lock_rotation(other._lock_rotation) {
}

PhysicsBody2D &PhysicsBody2D::operator=(const PhysicsBody2D &other) {
	if (this != &other) {
		CollisionObject2D::operator=(other);
		_linearVel = other._linearVel;
		_angularVel = other._angularVel;
		_linearAcc = other._linearAcc;
		_angularAcc = other._angularAcc;
		_mass = other._mass;
		_friction = other._friction;
		_restitution = other._restitution;
		_is_static = other._is_static;
		_lock_rotation = other._lock_rotation;
	}
	return *this;
}

PhysicsBody2D::~PhysicsBody2D() {
}

void PhysicsBody2D::applyForce(const raylib::Vector2 &force) noexcept {
	_linearAcc += force / _mass;
}

void PhysicsBody2D::moveTowards(const raylib::Vector2 &target,
								const float			   strength) noexcept {
	const raylib::Vector2 direction = target - _pos;
	_linearAcc += (direction.Normalize() * strength);
}

void PhysicsBody2D::moveAwayFrom(const raylib::Vector2 &target,
								 const float			strength) noexcept {
	const raylib::Vector2 direction = _pos - target;
	_linearAcc += (direction.Normalize() * strength);
}

void PhysicsBody2D::applyGravity(const float strength) noexcept {
	_linearAcc.y += strength;
}

void PhysicsBody2D::applyNewtonianGravity(const raylib::Vector2 &target,
										  const float strength) noexcept {
	const raylib::Vector2 direction = target - _pos;
	const float			  distance = direction.Length();
	if (distance > 0) {
		const float forceMagnitude = strength / (distance * distance);
		_linearAcc += direction.Normalize() * forceMagnitude;
	}
}

void PhysicsBody2D::applyFriction(const float strength) noexcept {
	if (_linearVel.Length() > 0) {
		const raylib::Vector2 frictionForce =
			_linearVel.Normalize() * -strength * _mass;
		applyForce(frictionForce);
	}
}

void PhysicsBody2D::collideWith(PhysicsBody2D &other,
								const float	   restitution) noexcept {
	if (_collisionShape.getShape()->getClassName() == "CircleShape2D" &&
		other._collisionShape.getShape()->getClassName() == "CircleShape2D") {
		collissionCircleCircle(other, restitution);
	} else if (_collisionShape.getShape()->getClassName() == "CircleShape2D" &&
			   other._collisionShape.getShape()->getClassName() ==
				   "RectangleShape2D") {
		collissionCircleRectangle(other, restitution);
	} else if (_collisionShape.getShape()->getClassName() ==
				   "RectangleShape2D" &&
			   other._collisionShape.getShape()->getClassName() ==
				   "RectangleShape2D") {
		collissionRectangleRectangle(other, restitution);
	}
}

void PhysicsBody2D::collissionCircleCircle(PhysicsBody2D &other,
										   const float restitution) noexcept {
	// Claude implement circle-circle collision response here
}

void PhysicsBody2D::collissionCircleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	// Claude implement circle-rectangle collision response here
}

void PhysicsBody2D::collissionRectangleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	// Claude implement rectangle-rectangle collision response here
}

raylib::Vector2 PhysicsBody2D::getLinearVel() const noexcept {
	return _linearVel;
}

void PhysicsBody2D::setLinearVel(const raylib::Vector2 &newLinearVel) noexcept {
	_linearVel = newLinearVel;
}

raylib::Vector2 PhysicsBody2D::getAngularVel() const noexcept {
	return _angularVel;
}

void PhysicsBody2D::setAngularVel(
	const raylib::Vector2 &newAngularVel) noexcept {
	_angularVel = newAngularVel;
}

raylib::Vector2 PhysicsBody2D::getLinearAcc() const noexcept {
	return _linearAcc;
}

void PhysicsBody2D::setLinearAcc(const raylib::Vector2 &newLinearAcc) noexcept {
	_linearAcc = newLinearAcc;
}

raylib::Vector2 PhysicsBody2D::getAngularAcc() const noexcept {
	return _angularAcc;
}

void PhysicsBody2D::setAngularAcc(
	const raylib::Vector2 &newAngularAcc) noexcept {
	_angularAcc = newAngularAcc;
}

float PhysicsBody2D::getMass() const noexcept {
	return _mass;
}

void PhysicsBody2D::setMass(const float newMass) noexcept {
	_mass = newMass;
}

float PhysicsBody2D::getFriction() const noexcept {
	return _friction;
}

void PhysicsBody2D::setFriction(const float newFriction) noexcept {
	_friction = newFriction;
}

float PhysicsBody2D::getRestitution() const noexcept {
	return _restitution;
}

void PhysicsBody2D::setRestitution(const float newRestitution) noexcept {
	_restitution = newRestitution;
}

bool PhysicsBody2D::isStatic() const noexcept {
	return _is_static;
}

void PhysicsBody2D::setStatic(const bool newIsStatic) noexcept {
	_is_static = newIsStatic;
}

bool PhysicsBody2D::isRotationLocked() const noexcept {
	return _lock_rotation;
}

void PhysicsBody2D::setLockRotation(const bool newLockRotation) noexcept {
	_lock_rotation = newLockRotation;
}

const std::string &PhysicsBody2D::getClassName() const noexcept {
	static const std::string className("PhysicsBody2D");
	return className;
}
