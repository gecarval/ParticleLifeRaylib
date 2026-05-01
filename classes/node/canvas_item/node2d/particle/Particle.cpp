#include "Particle.hpp"

const float Particle::_defaultSize = 5.0f;

Particle::Particle(const raylib::Vector2 &pos)
	: Object(), _pos(pos), _col(raylib::Color::White()), _rad(_defaultSize) {
}

Particle::Particle(const raylib::Color &col, const raylib::Vector2 &pos)
	: Object(), _pos(pos), _col(col), _rad(_defaultSize) {
}

Particle::Particle(const Particle &other)
	: Object(other), _pos(other._pos), _vel(other._vel), _acc(other._acc),
	  _col(other._col), _rad(other._rad) {
}

Particle &Particle::operator=(const Particle &other) {
	if (this != &other) {
		Object::operator=(other);
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

void Particle::draw() const noexcept {
	_pos.DrawCircle(_rad, _col);
}

void Particle::draw(const raylib::Window   &window,
					const raylib::Camera2D &camera) const noexcept {
	const raylib::Rectangle screenRect(
		camera.GetTarget().x - (window.GetWidth() / 2.0f) / camera.GetZoom(),
		camera.GetTarget().y - (window.GetHeight() / 2.0f) / camera.GetZoom(),
		window.GetWidth() / camera.GetZoom(),
		window.GetHeight() / camera.GetZoom());
	if (screenRect.CheckCollision(_pos, _rad)) {
		draw();
	}
}

void Particle::debugDraw() const noexcept {
	_pos.DrawCircleLine(_rad, raylib::Color::Red());
	_pos.DrawLine(_pos + _vel, raylib::Color::Green());
	_pos.DrawLine(_pos + _acc, raylib::Color::Blue());
}

void Particle::update() noexcept {
	const float deltaTime = raylib::Window::GetFrameTime();
	_vel += _acc * deltaTime;
	_pos += _vel * deltaTime;
	_acc = raylib::Vector2::Zero();
}

void Particle::applyForce(const raylib::Vector2 &force) noexcept {
	_acc += force;
}

void Particle::moveTowards(const raylib::Vector2 &target,
						   const float			  strength) noexcept {
	const raylib::Vector2 direction = target - _pos;
	_acc += (direction.Normalize() * strength);
}

void Particle::moveAwayFrom(const raylib::Vector2 &target,
							const float			   strength) noexcept {
	const raylib::Vector2 direction = _pos - target;
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
	const float			  distanceSqr = direction.LengthSqr();
	const float			  minDistance = _rad + other._rad;

	if (distanceSqr > 0.01f && distanceSqr < (minDistance * minDistance)) {
		const float		distance = sqrtf(distanceSqr);
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

const raylib::Vector2 &Particle::getPos() const noexcept {
	return _pos;
}

raylib::Vector2 &Particle::getPos() noexcept {
	return _pos;
}

void Particle::setPos(const raylib::Vector2 &newPos) noexcept {
	_pos = newPos;
}

const raylib::Vector2 &Particle::getVel() const noexcept {
	return _vel;
}

raylib::Vector2 &Particle::getVel() noexcept {
	return _vel;
}

void Particle::setVel(const raylib::Vector2 &newVel) noexcept {
	_vel = newVel;
}

const raylib::Vector2 &Particle::getAcc() const noexcept {
	return _acc;
}

raylib::Vector2 &Particle::getAcc() noexcept {
	return _acc;
}

void Particle::setAcc(const raylib::Vector2 &newAcc) noexcept {
	_acc = newAcc;
}

const raylib::Color &Particle::getCol() const noexcept {
	return _col;
}

raylib::Color &Particle::getCol() noexcept {
	return _col;
}

void Particle::setCol(const raylib::Color &newCol) noexcept {
	_col = newCol;
}

unsigned int Particle::getRadius() const noexcept {
	return _rad;
}

void Particle::setRadius(unsigned int newRad) noexcept {
	_rad = newRad;
}

const std::string &Particle::getClassName() const noexcept {
	static const std::string className("Particle");
	return (className);
}
