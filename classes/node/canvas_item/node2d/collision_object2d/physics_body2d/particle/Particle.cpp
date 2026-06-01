#include "Particle.hpp"

const float Particle::_defaultSize = 5.0f;

Particle::Particle(const std::string &instanceName, const raylib::Vector2 &pos,
				   const raylib::Color &col)
	: PhysicsBody2D(instanceName) {
	Sprite2D		 &sprite = *new Sprite2D("ParticleSprite");
	CollisionShape2D &collisionShape =
		*new CollisionShape2D("ParticleCollisionShape");
	pushBackChild(sprite.setColor(col).setShape(_defaultSize));
	pushBackChild(collisionShape.setShape(_defaultSize));
	setPos(pos);
}

Particle::Particle(const Particle &other) : PhysicsBody2D(other) {
}

Particle &Particle::operator=(const Particle &other) {
	if (this != &other) {
		PhysicsBody2D::operator=(other);
	}
	return *this;
}

Particle::~Particle() {
}

const std::string &Particle::getClassName() const noexcept {
	static const std::string className("Particle");
	return (className);
}
