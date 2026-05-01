#include "PhysicsServer.hpp"
#include "../node/canvas_item/node2d/particle/Particle.hpp"
#include <array>

// Static
const float	   PhysicsServer::CELL_SIZE = 10.0f;
PhysicsServer *PhysicsServer::_instance = nullptr;

// Pre-bucket the map so the first rebuild rarely triggers a rehash.
PhysicsServer::PhysicsServer() : Object() {
	_hashMap.reserve(4096);
}

PhysicsServer::~PhysicsServer() {
}

inline Vector2i
PhysicsServer::hashFunction(const raylib::Vector2 &position) const noexcept {
	return {static_cast<int>(position.x / CELL_SIZE),
			static_cast<int>(position.y / CELL_SIZE)};
}

PhysicsServer &PhysicsServer::getInstance() noexcept {
	if (_instance == nullptr) {
		_instance = new PhysicsServer();
	}
	return *_instance;
}

void PhysicsServer::deleteInstance() noexcept {
	if (_instance != nullptr) {
		delete _instance;
		_instance = nullptr;
	}
}

// Core API
void PhysicsServer::rebuild(std::vector<Particle *> &particles) {
	// Reuse allocated buckets — clear() keeps capacity.
	for (auto &[key, vec] : _hashMap) {
		vec.clear();
	}
	for (Particle *p : particles) {
		auto &bucket = _hashMap[hashFunction(p->getPos())];
		bucket.push_back(p);
	}
}

void PhysicsServer::getCollisions(Particle				  *particle,
								  std::vector<Particle *> &out) const {
	const Vector2i key = hashFunction(particle->getPos());
	// Offsets for the 3×3 neighbourhood (center cell + 8 surrounding).
	static constexpr std::array<std::pair<int, int>, 9> offsets{{
		{-1, -1},
		{0, -1},
		{1, -1},
		{-1, 0},
		{0, 0},
		{1, 0},
		{-1, 1},
		{0, 1},
		{1, 1},
	}};

	for (const auto &[dx, dy] : offsets) {
		const Vector2i neighborKey(key.x + dx, key.y + dy);
		const auto	   it = _hashMap.find(neighborKey);
		if (it == _hashMap.end()) {
			continue;
		}
		for (Particle *other : it->second) {
			if (*particle != *other) {
				out.push_back(other);
			}
		}
	}
}

// Legacy
void PhysicsServer::addParticles(std::vector<Particle *> &particles) {
	for (Particle *p : particles) {
		_hashMap[hashFunction(p->getPos())].push_back(p);
	}
}

std::vector<Particle *> PhysicsServer::getCollisions(Particle *particle) const {
	std::vector<Particle *> out;
	// Rough upper-bound guess to avoid repeated small reallocations.
	out.reserve(32);
	getCollisions(particle, out);
	return out;
}

void PhysicsServer::clear() {
	for (auto &[key, vec] : _hashMap) {
		vec.clear();
	}
}

const std::string &PhysicsServer::getClassName() const noexcept {
	static const std::string className("PhysicsServer");
	return (className);
}
