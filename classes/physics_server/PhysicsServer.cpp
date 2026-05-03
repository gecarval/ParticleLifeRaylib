#include "PhysicsServer.hpp"
#include "../node/canvas_item/node2d/collision_object2d/CollisionObject2D.hpp"
#include <array>

// Static
const float	   PhysicsServer::CELL_SIZE = 10.0f;
PhysicsServer *PhysicsServer::_instance = nullptr;
std::unordered_map<unsigned long, CollisionObject2D *>
	PhysicsServer::_collisionObjects;

// Pre-bucket the map so the first rebuild rarely triggers a rehash.
PhysicsServer::PhysicsServer(const std::string &instanceName)
	: Object(instanceName) {
	_hashMap.reserve(4096);
}

PhysicsServer::~PhysicsServer() {
}

inline Vector2i
PhysicsServer::hashFunction(const raylib::Vector2 &position) const noexcept {
	return position / CELL_SIZE;
}

void PhysicsServer::addCollisionObject(
	CollisionObject2D *collisionObject) noexcept {
	_collisionObjects[collisionObject->getInstanceID()] = collisionObject;
}

void PhysicsServer::removeCollisionObject(
	CollisionObject2D *collisionObject) noexcept {
	_collisionObjects.erase(collisionObject->getInstanceID());
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
	for (auto &[key, b] : _collisionObjects) {
		auto &bucket = _hashMap[hashFunction(b->getPos())];
		bucket.push_back(b);
	}
}

void PhysicsServer::getCollisions(CollisionObject2D &collisionObject,
								  std::vector<CollisionObject2D *> &out) const {
	const Vector2i key = hashFunction(collisionObject.getPos());
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
		for (CollisionObject2D *other : it->second) {
			if (collisionObject != *other) {
				out.push_back(other);
			}
		}
	}
}

// Legacy
std::vector<CollisionObject2D *>
PhysicsServer::getCollisions(CollisionObject2D &collisionObject) const {
	std::vector<CollisionObject2D *> out;
	// Rough upper-bound guess to avoid repeated small reallocations.
	out.reserve(32);
	getCollisions(collisionObject, out);
	return out;
}

void PhysicsServer::clear() noexcept {
	// Reuse allocated buckets — clear() keeps capacity.
	for (auto &[key, vec] : _hashMap) {
		vec.clear();
	}
}

const std::string &PhysicsServer::getClassName() const noexcept {
	static const std::string className("PhysicsServer");
	return (className);
}
