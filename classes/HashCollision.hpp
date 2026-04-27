#ifndef HASHCOLLISION_HPP
#define HASHCOLLISION_HPP

#include "../include/raylib-cpp.hpp"
#include <map>
#include <vector>

class Particle;
struct Vector2i;

class HashCollision {
  private:
	static const float	  _hashQuadSize;
	static HashCollision *_instance;

	std::map<Vector2i, std::vector<Particle *>> _hashMap;

	HashCollision();
	HashCollision(const HashCollision &other);
	HashCollision &operator=(const HashCollision &other);
	~HashCollision();

	Vector2i hashFunction(const raylib::Vector2 &position);

  public:
	static HashCollision   *getInstance();
	static void				deleteInstance();
	void					addParticles(std::vector<Particle *> &particles);
	std::vector<Particle *> getCollisions(Particle *particle);
	void					clear();
};

struct Vector2i {
	int x;
	int y;

	Vector2i(int x = 0, int y = 0) : x(x), y(y) {
	}

	Vector2i(const raylib::Vector2 &vec)
		: x(static_cast<int>(vec.x)), y(static_cast<int>(vec.y)) {
	}

	Vector2i(const Vector2i &other) : x(other.x), y(other.y) {
	}

	Vector2i &operator=(const Vector2i &other) {
		if (this != &other) {
			x = other.x;
			y = other.y;
		}
		return *this;
	}

	~Vector2i() {
	}

	bool operator==(const Vector2i &other) const {
		return x == other.x && y == other.y;
	}

	bool operator!=(const Vector2i &other) const {
		return !(*this == other);
	}

	bool operator<(const Vector2i &other) const {
		return (x < other.x) || (x == other.x && y < other.y);
	}
};

#endif // HASHCOLLISION_HPP
