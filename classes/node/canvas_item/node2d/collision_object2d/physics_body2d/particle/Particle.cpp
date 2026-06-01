#include "Particle.hpp"

const float Particle::_defaultSize = 5.0f;

Particle::Particle(const std::string &instanceName, const raylib::Vector2 &pos,
				   const raylib::Color &col)
	: PhysicsBody2D(instanceName) {
	Sprite2D		 &sprite = *new Sprite2D("ParticleSprite");
	CollisionShape2D &collisionShape =
		*new CollisionShape2D("ParticleCollisionShape");
	sprite.setColor(col);
	sprite.setShape(_defaultSize);
	collisionShape.setShape(_defaultSize);
	pushBackChild(sprite);
	pushBackChild(collisionShape);
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

void Particle::draw() const noexcept {
	auto children = getChildren();
	for (const Node *child : children) {
		const Node2D *node = dynamic_cast<const Node2D *>(child);
		if (node != nullptr) {
			node->draw();
		}
	}
}

const std::string &Particle::getClassName() const noexcept {
	static const std::string className("Particle");
	return (className);
}
