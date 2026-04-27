#include "HashCollision.hpp"
#include "Particle.hpp"

const float	   HashCollision::_hashQuadSize = 10.0f;
HashCollision *HashCollision::_instance = nullptr;

HashCollision::HashCollision() : _hashMap() {
}

HashCollision::HashCollision(const HashCollision &other)
	: _hashMap(other._hashMap) {
}

HashCollision &HashCollision::operator=(const HashCollision &other) {
	if (this != &other) {
		_hashMap = other._hashMap;
	}
	return *this;
}

HashCollision::~HashCollision() {
}

Vector2i HashCollision::hashFunction(const raylib::Vector2 &position) {
	return Vector2i(static_cast<int>(position.x / _hashQuadSize),
					static_cast<int>(position.y / _hashQuadSize));
}

HashCollision *HashCollision::getInstance() {
	if (_instance == nullptr) {
		_instance = new HashCollision();
	}
	return _instance;
}

void HashCollision::deleteInstance() {
	if (_instance != nullptr) {
		delete _instance;
		_instance = nullptr;
	}
}

void HashCollision::addParticles(std::vector<Particle *> &particles) {
	for (Particle *particle : particles) {
		Vector2i hashKey = hashFunction(particle->getPos());
		_hashMap[hashKey].push_back(particle);
	}
}

std::vector<Particle *> HashCollision::getCollisions(Particle *particle) {
	Vector2i				hashKey = hashFunction(particle->getPos());
	std::vector<Particle *> collisions;

	if (_hashMap.find(hashKey) != _hashMap.end()) {
		collisions.reserve(_hashMap[hashKey].size());
		for (Particle *other : _hashMap[hashKey]) {
			if (other != particle) {
				collisions.push_back(other);
			}
		}
	}

	const std::vector<Vector2i> neighbors = {
		Vector2i(hashKey.x - 1, hashKey.y - 1),
		Vector2i(hashKey.x, hashKey.y - 1),
		Vector2i(hashKey.x + 1, hashKey.y - 1),
		Vector2i(hashKey.x - 1, hashKey.y),
		Vector2i(hashKey.x + 1, hashKey.y),
		Vector2i(hashKey.x - 1, hashKey.y + 1),
		Vector2i(hashKey.x, hashKey.y + 1),
		Vector2i(hashKey.x + 1, hashKey.y + 1)};

	const raylib::Vector2 size(_hashQuadSize, _hashQuadSize);
	for (const Vector2i &neighborKey : neighbors) {
		if (_hashMap.find(neighborKey) != _hashMap.end()) {
			raylib::Vector2 neighborPos = raylib::Vector2(
				neighborKey.x * _hashQuadSize, neighborKey.y * _hashQuadSize);
			raylib::Rectangle neighborRect(neighborPos, size);
			if (!neighborRect.CheckCollision(particle->getPos(),
											 particle->getRadius())) {
				continue;
			}
			collisions.reserve(collisions.size() +
							   _hashMap[neighborKey].size());
			for (Particle *other : _hashMap[neighborKey]) {
				if (other != particle) {
					collisions.push_back(other);
				}
			}
		}
	}

	return collisions;
}

void HashCollision::clear() {
	_hashMap.clear();
}
