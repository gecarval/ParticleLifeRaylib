#include "PhysicsServer.hpp"
#include "../node/canvas_item/node2d/Node2D.hpp"
#include <array>

// Static
const float	   PhysicsServer::CELL_SIZE = 10.0f;
PhysicsServer *PhysicsServer::_instance = nullptr;

// Pre-bucket the map so the first rebuild rarely triggers a rehash.
PhysicsServer::PhysicsServer(const std::string &instanceName)
	: Object(instanceName), _physicsBodies(nullptr) {
	_hashMap.reserve(4096);
}

PhysicsServer::~PhysicsServer() {
}

inline Vector2i
PhysicsServer::hashFunction(const raylib::Vector2 &position) const noexcept {
	return position / CELL_SIZE;
}

PhysicsServer &PhysicsServer::getInstance() noexcept {
	if (_instance == nullptr) {
		_instance = new PhysicsServer("PhysicsServer");
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
void PhysicsServer::rebuild() {
	clear();
	if (_physicsBodies == nullptr) {
		return;
	}
	for (Node2D *b : *_physicsBodies) {
		auto &bucket = _hashMap[hashFunction(b->getPos())];
		bucket.push_back(b);
	}
}

void PhysicsServer::getCollisions(Node2D				&physicsBody,
								  std::vector<Node2D *> &out) const {
	const Vector2i key = hashFunction(physicsBody.getPos());
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
		for (Node2D *other : it->second) {
			if (physicsBody != *other) {
				out.push_back(other);
			}
		}
	}
}

// Legacy
void PhysicsServer::setPhysicsBodies(std::vector<Node2D *> &physicsBodies) {
	_physicsBodies = &physicsBodies;
}

std::vector<Node2D *> PhysicsServer::getCollisions(Node2D &physicsBody) const {
	std::vector<Node2D *> out;
	// Rough upper-bound guess to avoid repeated small reallocations.
	out.reserve(32);
	getCollisions(physicsBody, out);
	return out;
}

void PhysicsServer::clear() {
	// Reuse allocated buckets — clear() keeps capacity.
	for (auto &[key, vec] : _hashMap) {
		vec.clear();
	}
}

const std::string &PhysicsServer::getClassName() const noexcept {
	static const std::string className("PhysicsServer");
	return (className);
}
