#include "PhysicsBody2D.hpp"
#include "../../../../../shape2d/circle_shape2d/CircleShape2D.hpp"
#include "../../../../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"
#include <algorithm>

PhysicsBody2D::PhysicsBody2D(const std::string &instanceName)
	: CollisionObject2D(instanceName), _linearVel(0, 0), _angularVel(0, 0),
	  _linearAcc(0, 0), _angularAcc(0, 0), _mass(1.0f), _friction(0.1f),
	  _restitution(0.5f), _is_static(false), _lock_rotation(true) {
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

	// Integrate linear motion
	_linearVel += _linearAcc * deltaTime;
	_linearAcc = raylib::Vector2::Zero();
	setPos(_pos + _linearVel * deltaTime);

	// Integrate angular motion.
	// angularVel/Acc store the scalar (rad/s, rad/s²) in the .x component;
	// .y is unused and kept at 0 for consistency with the Vector2 type.
	if (!_lock_rotation) {
		_angularVel.x += _angularAcc.x * deltaTime;
		_angularAcc.x = 0.0f;
		setRotation(_rot + _angularVel.x * deltaTime);
	} else {
		_angularVel.x = 0.0f;
		_angularAcc.x = 0.0f;
	}
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

// ---------------------------------------------------------------------------
// Helper: 2D scalar cross product  a × b = a.x*b.y - a.y*b.x
// ---------------------------------------------------------------------------
static inline float cross2D(const raylib::Vector2 &a,
							const raylib::Vector2 &b) noexcept {
	return a.x * b.y - a.y * b.x;
}

// ---------------------------------------------------------------------------
// Helper: perpendicular (CCW rotation 90°) of v — used to convert a scalar
// angular velocity into a linear velocity at a contact arm r:
//   v_at_contact = omega * perp(r)   where perp(r) = (-r.y, r.x)
// ---------------------------------------------------------------------------
static inline raylib::Vector2 perpCCW(const raylib::Vector2 &v) noexcept {
	return raylib::Vector2(-v.y, v.x);
}

// ---------------------------------------------------------------------------
// resolveContact — shared impulse solver used by all three collision methods.
//
//  other      — the other body
//  normal     — unit collision normal pointing FROM other INTO this
//  contactPt  — world-space contact point
//  overlap    — penetration depth along the normal
//  inertiaA   — moment of inertia of *this  (pass 0 if static)
//  inertiaB   — moment of inertia of other  (pass 0 if static)
//  restitution— coefficient of restitution [0,1]
// ---------------------------------------------------------------------------
void PhysicsBody2D::resolveContact(PhysicsBody2D		 &other,
								   const raylib::Vector2 &normal,
								   const raylib::Vector2 &contactPt,
								   const float overlap, const float inertiaA,
								   const float inertiaB,
								   const float restitution) noexcept {
	const float invMassA = _is_static ? 0.0f : 1.0f / _mass;
	const float invMassB = other._is_static ? 0.0f : 1.0f / other._mass;
	const float totalInvMass = invMassA + invMassB;

	// --- Positional correction (prevent overlap) ---------------------------
	// Each body absorbs a share of the correction proportional to its
	// inverse mass (lighter bodies move more).
	if (totalInvMass > 0.0f) {
		const float correctionDepth = overlap / totalInvMass;
		if (!_is_static) setPos(_pos + normal * (correctionDepth * invMassA));
		if (!other._is_static)
			other.setPos(other._pos - normal * (correctionDepth * invMassB));
	}

	// --- Lever arms (centre of mass → contact point) ----------------------
	const raylib::Vector2 rA = contactPt - _pos;
	const raylib::Vector2 rB = contactPt - other._pos;

	// --- Velocity at the contact point ------------------------------------
	// v_contact = v_linear + omega × r
	// In 2D: omega × r = omega_scalar * perp(r) = omega * (-r.y, r.x)
	const float omegaA = _lock_rotation ? 0.0f : _angularVel.x;
	const float omegaB = other._lock_rotation ? 0.0f : other._angularVel.x;

	const raylib::Vector2 velAtContactA = _linearVel + perpCCW(rA) * omegaA;
	const raylib::Vector2 velAtContactB =
		other._linearVel + perpCCW(rB) * omegaB;

	const raylib::Vector2 relVel = velAtContactA - velAtContactB;
	const float			  velAlongNormal = relVel.DotProduct(normal);

	// Only resolve if the bodies are approaching each other
	if (velAlongNormal >= 0.0f) return;

	// --- Impulse magnitude ------------------------------------------------
	// Standard rigid-body impulse formula including rotational terms:
	//
	//          -(1 + e) * (v_rel · n)
	//   j = ─────────────────────────────────────────────────────
	//        1/mA + 1/mB + (rA × n)²/IA + (rB × n)²/IB
	//
	const float invInertiaA =
		(_lock_rotation || inertiaA <= 0.0f) ? 0.0f : 1.0f / inertiaA;
	const float invInertiaB =
		(other._lock_rotation || inertiaB <= 0.0f) ? 0.0f : 1.0f / inertiaB;

	const float rACrossN = cross2D(rA, normal);
	const float rBCrossN = cross2D(rB, normal);

	const float denom = totalInvMass + rACrossN * rACrossN * invInertiaA +
						rBCrossN * rBCrossN * invInertiaB;

	if (denom <= 0.0f) return;

	const float impulseMag = -(1.0f + restitution) * velAlongNormal / denom;
	const raylib::Vector2 impulse = normal * impulseMag;

	// --- Apply linear impulse ---------------------------------------------
	if (!_is_static) _linearVel += impulse * invMassA;
	if (!other._is_static) other._linearVel -= impulse * invMassB;

	// --- Apply angular impulse  delta_omega = (r × J) / I ----------------
	if (!_lock_rotation && invInertiaA > 0.0f)
		_angularVel.x += cross2D(rA, impulse) * invInertiaA;
	if (!other._lock_rotation && invInertiaB > 0.0f)
		other._angularVel.x -= cross2D(rB, impulse) * invInertiaB;
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

	if (_collisionShape.getShape() == nullptr ||
		otherCollisionShape.getShape() == nullptr) {
		return;
	}

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
	if (colliders.empty()) return;
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) return;

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	const CircleShape2D *myCircle =
		dynamic_cast<const CircleShape2D *>(_collisionShape.getShape());
	const CircleShape2D *otherCircle =
		dynamic_cast<const CircleShape2D *>(otherCollisionShape.getShape());
	if (!myCircle || !otherCircle) return;

	const raylib::Vector2 delta = _pos - other._pos;
	const float			  dist = delta.Length();
	const float minDist = myCircle->getRadius() + otherCircle->getRadius();

	if (dist <= 0.0f || dist >= minDist) return;

	// Unit normal pointing from other → this
	const raylib::Vector2 normal = delta / dist;
	const float			  overlap = minDist - dist;

	// Contact point lies on the surface of the other circle along the normal
	const raylib::Vector2 contactPt =
		other._pos + normal * otherCircle->getRadius();

	// Moment of inertia for a solid disc: I = 0.5 * m * r²
	const float inertiaA =
		0.5f * _mass * myCircle->getRadius() * myCircle->getRadius();
	const float inertiaB = 0.5f * other._mass * otherCircle->getRadius() *
						   otherCircle->getRadius();

	resolveContact(other, normal, contactPt, overlap, inertiaA, inertiaB,
				   restitution);
}

void PhysicsBody2D::collissionRectangleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) return;
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) return;

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	const RectangleShape2D *myRect =
		dynamic_cast<const RectangleShape2D *>(_collisionShape.getShape());
	const RectangleShape2D *otherRect =
		dynamic_cast<const RectangleShape2D *>(otherCollisionShape.getShape());
	if (!myRect || !otherRect) return;

	// AABB overlap — SAT for axis-aligned boxes
	const raylib::Vector2 myMin = _pos;
	const raylib::Vector2 myMax = _pos + myRect->getSize();
	const raylib::Vector2 otherMin = other._pos;
	const raylib::Vector2 otherMax = other._pos + otherRect->getSize();

	const float overlapX =
		std::min(myMax.x, otherMax.x) - std::max(myMin.x, otherMin.x);
	const float overlapY =
		std::min(myMax.y, otherMax.y) - std::max(myMin.y, otherMin.y);

	if (overlapX <= 0.0f || overlapY <= 0.0f) return;

	// Resolve along the axis of least penetration
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

	// Contact point: centre of the overlapping region
	const raylib::Vector2 contactPt(
		(std::max(myMin.x, otherMin.x) + std::min(myMax.x, otherMax.x)) * 0.5f,
		(std::max(myMin.y, otherMin.y) + std::min(myMax.y, otherMax.y)) * 0.5f);

	// Moment of inertia for a rectangle: I = m * (w² + h²) / 12
	const raylib::Vector2 mySize = myRect->getSize();
	const raylib::Vector2 otherSize = otherRect->getSize();
	const float			  inertiaA =
		_mass * (mySize.x * mySize.x + mySize.y * mySize.y) / 12.0f;
	const float inertiaB =
		other._mass * (otherSize.x * otherSize.x + otherSize.y * otherSize.y) /
		12.0f;

	resolveContact(other, normal, contactPt, overlap, inertiaA, inertiaB,
				   restitution);
}

void PhysicsBody2D::collissionCircleRectangle(
	PhysicsBody2D &other, const float restitution) noexcept {
	auto colliders = findClass("CollisionShape2D");
	if (colliders.empty()) return;
	auto otherColliders = other.findClass("CollisionShape2D");
	if (otherColliders.empty()) return;

	const CollisionShape2D &_collisionShape =
		dynamic_cast<CollisionShape2D &>(*colliders[0]);
	const CollisionShape2D &otherCollisionShape =
		dynamic_cast<CollisionShape2D &>(*otherColliders[0]);

	// `this` is the circle, `other` is the rectangle.
	const CircleShape2D *myCircle =
		dynamic_cast<const CircleShape2D *>(_collisionShape.getShape());
	const RectangleShape2D *otherRect =
		dynamic_cast<const RectangleShape2D *>(otherCollisionShape.getShape());
	if (!myCircle || !otherRect) return;

	// Closest point on the AABB to the circle centre
	const float clampedX = std::max(
		other._pos.x, std::min(_pos.x, other._pos.x + otherRect->getSize().x));
	const float clampedY = std::max(
		other._pos.y, std::min(_pos.y, other._pos.y + otherRect->getSize().y));

	const raylib::Vector2 contactPt(clampedX, clampedY);
	const raylib::Vector2 delta = _pos - contactPt;
	const float			  dist = delta.Length();

	if (dist <= 0.0f || dist >= myCircle->getRadius()) return;

	const raylib::Vector2 normal =
		(dist > 0.0f) ? (delta / dist) : raylib::Vector2(0, -1);
	const float overlap = myCircle->getRadius() - dist;

	// Circle: I = 0.5 * m * r²     Rectangle: I = m * (w² + h²) / 12
	const float inertiaA =
		0.5f * _mass * myCircle->getRadius() * myCircle->getRadius();
	const raylib::Vector2 otherSize = otherRect->getSize();
	const float			  inertiaB =
		other._mass * (otherSize.x * otherSize.x + otherSize.y * otherSize.y) /
		12.0f;

	resolveContact(other, normal, contactPt, overlap, inertiaA, inertiaB,
				   restitution);
}

void PhysicsBody2D::collissionRectangleCircle(
	PhysicsBody2D &other, const float restitution) noexcept {
	other.collissionCircleRectangle(*this, restitution);
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
