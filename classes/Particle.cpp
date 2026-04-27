#include "Particle.hpp"

const float	  Particle::_defaultSize = 5.0f;
unsigned long Particle::_instanceIDCounter = 0;

Particle::Particle(const raylib::Vector2 &pos)
	: _instanceID(_instanceIDCounter++), _pos(pos),
	  _col(raylib::Color::White()), _rad(_defaultSize) {
}

Particle::Particle(const raylib::Color &col, const raylib::Vector2 &pos)
	: _instanceID(_instanceIDCounter++), _pos(pos), _col(col),
	  _rad(_defaultSize) {
}

Particle::Particle(const Particle &other)
	: _instanceID(_instanceIDCounter++), _pos(other._pos), _vel(other._vel),
	  _acc(other._acc), _col(other._col), _rad(other._rad) {
}

Particle &Particle::operator=(const Particle &other) {
	if (this != &other) {
		_pos = other._pos;
		_vel = other._vel;
		_acc = other._acc;
		_col = other._col;
		_rad = other._rad;
	}
	return *this;
}

Particle::~Particle() {
}

void Particle::update(void) noexcept {
	const float deltaTime = raylib::Window::GetFrameTime();
	_vel += _acc * deltaTime;
	_pos += _vel * deltaTime;
	_acc = raylib::Vector2::Zero();
}

void Particle::draw(void) const noexcept {
	_pos.DrawCircle(_rad, _col);
}

void Particle::debugDraw(void) const noexcept {
	_pos.DrawCircleLine(_rad, raylib::Color::Red());
	_pos.DrawLine(_pos + _vel, raylib::Color::Green());
	_pos.DrawLine(_pos + _acc, raylib::Color::Blue());
}

void Particle::applyForce(const raylib::Vector2 &force) noexcept {
	_acc += force;
}

void Particle::moveTowards(const raylib::Vector2 &target,
						   const float			  strength) noexcept {
	raylib::Vector2 direction = target - _pos;
	_acc += (direction.Normalize() * strength);
}

void Particle::moveAwayFrom(const raylib::Vector2 &target,
							const float			   strength) noexcept {
	raylib::Vector2 direction = _pos - target;
	_acc += (direction.Normalize() * strength);
}

void Particle::applyGravity(const float strength) noexcept {
	_acc.y += strength;
}

void Particle::applyNewtonianGravity(const raylib::Vector2 &target,
									 const float			strength) noexcept {
	const raylib::Vector2 direction = target - _pos;
	const float			  distance = direction.Length();
	if (distance > 0) {
		const float forceMagnitude = strength / (distance * distance);
		_acc += direction.Normalize() * forceMagnitude;
	}
}

void Particle::applyFriction(const float strength) noexcept {
	_acc -= _vel * strength;
}

void Particle::collideWith(Particle &other, const float restitution) noexcept {
	const raylib::Vector2 direction = other._pos - _pos;
	const float			  distance = direction.Length();
	const float			  minDistance = _rad + other._rad;

	if (distance < minDistance && distance > 0) {
		raylib::Vector2 normal = direction.Normalize();
		const float relativeVelocity = (_vel - other._vel).DotProduct(normal);
		// Assuming equal mass
		const float impulseMagnitude =
			-(1 + restitution) * relativeVelocity / 2.0f;

		const raylib::Vector2 impulse = normal * impulseMagnitude;
		_vel += impulse;
		other._vel -= impulse;

		// Positional correction to prevent sinking
		const float			  penetrationDepth = minDistance - distance;
		const raylib::Vector2 correction = normal * (penetrationDepth / 2.0f);
		_pos -= correction;
		other._pos += correction;
	}
}

const raylib::Vector2 &Particle::getPos(void) const noexcept {
	return _pos;
}

raylib::Vector2 &Particle::getPos(void) noexcept {
	return _pos;
}

void Particle::setPos(const raylib::Vector2 &newPos) noexcept {
	_pos = newPos;
}

const raylib::Vector2 &Particle::getVel(void) const noexcept {
	return _vel;
}

raylib::Vector2 &Particle::getVel(void) noexcept {
	return _vel;
}

void Particle::setVel(const raylib::Vector2 &newVel) noexcept {
	_vel = newVel;
}

const raylib::Vector2 &Particle::getAcc(void) const noexcept {
	return _acc;
}

raylib::Vector2 &Particle::getAcc(void) noexcept {
	return _acc;
}

void Particle::setAcc(const raylib::Vector2 &newAcc) noexcept {
	_acc = newAcc;
}

const raylib::Color &Particle::getCol(void) const noexcept {
	return _col;
}

raylib::Color &Particle::getCol(void) noexcept {
	return _col;
}

void Particle::setCol(const raylib::Color &newCol) noexcept {
	_col = newCol;
}

unsigned int Particle::getRadius(void) const noexcept {
	return _rad;
}

void Particle::setRadius(unsigned int newRad) noexcept {
	_rad = newRad;
}

unsigned long Particle::getInstanceID(void) const noexcept {
	return (_instanceID);
}

bool Particle::operator==(const Particle &other) const noexcept {
	return (_instanceID == other._instanceID);
}

bool Particle::operator!=(const Particle &other) const noexcept {
	return (_instanceID != other._instanceID);
}
