#ifndef HASHCOLLISION_HPP
#define HASHCOLLISION_HPP

#include "../include/raylib-cpp.hpp"
#include "Object.hpp"
#include <unordered_map>
#include <vector>

class Particle;
struct Vector2i;
struct Vector2iHash {
	std::size_t operator()(const Vector2i &v) const noexcept;
};

struct Vector2i {
	int x;
	int y;
	constexpr Vector2i(int x = 0, int y = 0) noexcept : x(x), y(y) {
	}
	constexpr bool operator==(const Vector2i &other) const noexcept {
		return x == other.x && y == other.y;
	}
	constexpr bool operator!=(const Vector2i &other) const noexcept {
		return x != other.x || y != other.y;
	}
};

inline std::size_t Vector2iHash::operator()(const Vector2i &v) const noexcept {
	std::size_t h = static_cast<std::size_t>(v.x);
	h ^= static_cast<std::size_t>(v.y) + 0x9e3779b9u + (h << 6) + (h >> 2);
	return h;
}

class HashCollision : public Object {
  private:
	static const float	  CELL_SIZE;
	static HashCollision *_instance;

	std::unordered_map<Vector2i, std::vector<Particle *>, Vector2iHash>
		_hashMap;

	HashCollision();
	HashCollision(const HashCollision &) = delete;
	HashCollision &operator=(const HashCollision &) = delete;
	~HashCollision();

	inline Vector2i
	hashFunction(const raylib::Vector2 &position) const noexcept;

  public:
	static HashCollision *getInstance();
	static void			  deleteInstance();

	// Core API
	void rebuild(std::vector<Particle *> &particles);
	void getCollisions(Particle *particle, std::vector<Particle *> &out) const;

	// Legacy
	void					addParticles(std::vector<Particle *> &particles);
	std::vector<Particle *> getCollisions(Particle *particle) const;
	void					clear();

	virtual const std::string &getClassName(void) const noexcept;
};

#endif // HASHCOLLISION_HPP
