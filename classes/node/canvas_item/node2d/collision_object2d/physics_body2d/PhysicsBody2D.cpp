#include "PhysicsBody2D.hpp"
#include "../../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"
#include <algorithm>

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

void PhysicsBody2D::updatePhysics() noexcept {
	if (_is_static) {
		return;
	}
	const float deltaTime = raylib::Window::GetFrameTime();
	_linearVel += _linearAcc * deltaTime;
	_pos += _linearVel * deltaTime;
	_linearAcc = raylib::Vector2::Zero();
}

void PhysicsBody2D::drawDebug() const noexcept {
	if (_linearVel.Length() > 0.001f) {
		const raylib::Vector2 tip = _pos + _linearVel * 0.1f;
		DrawLineV({_pos.x, _pos.y}, {tip.x, tip.y}, ORANGE);
	}
	Node2D::drawDebug();
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
	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) {
		return;
	}
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) {
		return;
	}

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	if (_collisionShape.getShape()->getClassName() == "CircleShape2D" &&
		otherCollisionShape.getShape()->getClassName() == "CircleShape2D") {
		collissionCircleCircle(other, restitution);
	} else if (_collisionShape.getShape()->getClassName() == "CircleShape2D" &&
			   otherCollisionShape.getShape()->getClassName() ==
				   "RectangleShape2D") {
		collissionCircleRectangle(other, restitution);
	} else if (_collisionShape.getShape()->getClassName() ==
				   "RectangleShape2D" &&
			   otherCollisionShape.getShape()->getClassName() ==
				   "RectangleShape2D") {
		collissionRectangleRectangle(other, restitution);
	} else if (_collisionShape.getShape()->getClassName() ==
				   "RectangleShape2D" &&
			   otherCollisionShape.getShape()->getClassName() ==
				   "CircleShape2D") {
		other.collissionRectangleCircle(*this, restitution);
	}
}

void PhysicsBody2D::collissionCircleCircle(PhysicsBody2D &other,
										   const float restitution) noexcept {

	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) {
		return;
	}
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) {
		return;
	}

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	const CircleShape2D *myCircle =
		dynamic_cast<const CircleShape2D *>(_collisionShape.getShape());
	const CircleShape2D *otherCircle =
		dynamic_cast<const CircleShape2D *>(otherCollisionShape.getShape());
	if (!myCircle || !otherCircle) {
		return;
	}

	const raylib::Vector2 delta = _pos - other._pos;
	const float			  dist = delta.Length();
	const float minDist = myCircle->getRadius() + otherCircle->getRadius();

	if (dist <= 0.0f || dist >= minDist) {
		return;
	}

	// --- Positional correction (prevent overlap) ---------------------------
	const raylib::Vector2 normal = delta / dist; // unit normal A→B
	const float			  overlap = minDist - dist;
	const float totalInvMass = (_is_static ? 0.0f : 1.0f / _mass) +
							   (other._is_static ? 0.0f : 1.0f / other._mass);

	if (totalInvMass > 0.0f) {
		const float correctionScale = overlap / totalInvMass * 0.5f;
		if (!_is_static) {
			_pos += normal * (correctionScale / _mass);
		}
		if (!other._is_static) {
			other._pos -= normal * (correctionScale / other._mass);
		}
	}

	// --- Impulse-based velocity response ------------------------------------
	const raylib::Vector2 relVel = _linearVel - other._linearVel;
	const float			  velAlongNormal = relVel.DotProduct(normal);

	// Only resolve if objects are approaching
	if (velAlongNormal >= 0.0f) {
		return;
	}

	const float e = restitution;
	float		impulseMag = -(1.0f + e) * velAlongNormal;
	if (totalInvMass > 0.0f) {
		impulseMag /= totalInvMass;
	}

	const raylib::Vector2 impulse = normal * impulseMag;
	if (!_is_static) {
		_linearVel += impulse / _mass;
	}
	if (!other._is_static) {
		other._linearVel -= impulse / other._mass;
	}
}

void PhysicsBody2D::collissionRectangleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) {
		return;
	}
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) {
		return;
	}

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	const RectangleShape2D *myRect =
		dynamic_cast<const RectangleShape2D *>(_collisionShape.getShape());
	const RectangleShape2D *otherRect =
		dynamic_cast<const RectangleShape2D *>(otherCollisionShape.getShape());
	if (!myRect || !otherRect) {
		return;
	}

	// AABB overlap on each axis — SAT for axis-aligned boxes.
	const raylib::Vector2 myMin = _pos;
	const raylib::Vector2 myMax = _pos + myRect->getSize();
	const raylib::Vector2 otherMin = other._pos;
	const raylib::Vector2 otherMax = other._pos + otherRect->getSize();

	const float overlapX =
		std::min(myMax.x, otherMax.x) - std::max(myMin.x, otherMin.x);
	const float overlapY =
		std::min(myMax.y, otherMax.y) - std::max(myMin.y, otherMin.y);

	if (overlapX <= 0.0f || overlapY <= 0.0f) {
		return; // Not overlapping
	}

	// Resolve along the axis of least penetration.
	raylib::Vector2 normal;
	float			overlap;
	if (overlapX < overlapY) {
		overlap = overlapX;
		normal = (_pos.x < other._pos.x) ? raylib::Vector2(-1.0f, 0.0f)
										 : raylib::Vector2(1.0f, 0.0f);
	} else {
		overlap = overlapY;
		normal = (_pos.y < other._pos.y) ? raylib::Vector2(0.0f, -1.0f)
										 : raylib::Vector2(0.0f, 1.0f);
	}

	// --- Positional correction ---------------------------------------------
	const float totalInvMass = (_is_static ? 0.0f : 1.0f / _mass) +
							   (other._is_static ? 0.0f : 1.0f / other._mass);

	if (totalInvMass > 0.0f) {
		const float correctionScale = overlap / totalInvMass * 0.5f;
		if (!_is_static) {
			_pos += normal * (correctionScale / _mass);
		}
		if (!other._is_static) {
			other._pos -= normal * (correctionScale / other._mass);
		}
	}

	// --- Impulse response --------------------------------------------------
	const raylib::Vector2 relVel = _linearVel - other._linearVel;
	const float			  velAlongNormal = relVel.DotProduct(normal);
	if (velAlongNormal >= 0.0f) {
		return;
	}

	float impulseMag = -(1.0f + restitution) * velAlongNormal;
	if (totalInvMass > 0.0f) {
		impulseMag /= totalInvMass;
	}
	const raylib::Vector2 impulse = normal * impulseMag;
	if (!_is_static) {
		_linearVel += impulse / _mass;
	}
	if (!other._is_static) {
		other._linearVel -= impulse / other._mass;
	}
}

void PhysicsBody2D::collissionCircleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) {
		return;
	}
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) {
		return;
	}

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	// `this` is the circle, `other` is the rectangle.
	const CircleShape2D *myCircle =
		dynamic_cast<const CircleShape2D *>(_collisionShape.getShape());
	const RectangleShape2D *otherRect =
		dynamic_cast<const RectangleShape2D *>(otherCollisionShape.getShape());
	if (!myCircle || !otherRect) {
		return;
	}

	// Find the closest point on the AABB to the circle centre.
	const raylib::Vector2 circCentre = _pos;

	const float clampedX =
		std::max(other._pos.x,
				 std::min(circCentre.x, other._pos.x + otherRect->getSize().x));
	const float clampedY =
		std::max(other._pos.y,
				 std::min(circCentre.y, other._pos.y + otherRect->getSize().y));

	const raylib::Vector2 closest(clampedX, clampedY);
	const raylib::Vector2 delta = circCentre - closest;
	const float			  dist = delta.Length();

	if (dist <= 0.0f || dist >= myCircle->getRadius()) {
		return;
	}

	// --- Positional correction ---------------------------------------------
	const raylib::Vector2 normal =
		(dist > 0.0f) ? (delta / dist) : raylib::Vector2(0, -1);
	const float overlap = myCircle->getRadius() - dist;
	const float totalInvMass = (_is_static ? 0.0f : 1.0f / _mass) +
							   (other._is_static ? 0.0f : 1.0f / other._mass);

	if (totalInvMass > 0.0f) {
		const float correctionScale = overlap / totalInvMass * 0.5f;
		if (!_is_static) {
			_pos += normal * (correctionScale / _mass);
		}
		if (!other._is_static) {
			other._pos -= normal * (correctionScale / other._mass);
		}
	}

	// --- Impulse response ---------------------------------------------------
	const raylib::Vector2 relVel = _linearVel - other._linearVel;
	const float			  velAlongNormal = relVel.DotProduct(normal);
	if (velAlongNormal >= 0.0f) {
		return;
	}

	float impulseMag = -(1.0f + restitution) * velAlongNormal;
	if (totalInvMass > 0.0f) {
		impulseMag /= totalInvMass;
	}
	const raylib::Vector2 impulse = normal * impulseMag;
	if (!_is_static) {
		_linearVel += impulse / _mass;
	}
	if (!other._is_static) {
		other._linearVel -= impulse / other._mass;
	}
}

void PhysicsBody2D::collissionRectangleCircle(
	PhysicsBody2D &other, const float restitution) noexcept {
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
