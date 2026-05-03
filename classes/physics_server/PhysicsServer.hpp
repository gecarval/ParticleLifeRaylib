#ifndef PHYSICSSERVER_HPP
#define PHYSICSSERVER_HPP

#include "../../include/raylib-cpp.hpp"
#include "../Object.hpp"
#include <unordered_map>

class CollisionObject2D;
struct Vector2i {
	int x;
	int y;
	constexpr Vector2i(const raylib::Vector2 &v) noexcept
		: x(static_cast<int>(v.x)), y(static_cast<int>(v.y)) {
	}
	constexpr Vector2i(int x = 0, int y = 0) noexcept : x(x), y(y) {
	}
	constexpr bool operator==(const Vector2i &other) const noexcept {
		return x == other.x && y == other.y;
	}
	constexpr bool operator!=(const Vector2i &other) const noexcept {
		return x != other.x || y != other.y;
	}
};

struct Vector2iHash {
	inline std::size_t operator()(const Vector2i &v) const noexcept {
		std::size_t h = static_cast<std::size_t>(v.x);
		h ^= static_cast<std::size_t>(v.y) + 0x9e3779b9u + (h << 6) + (h >> 2);
		return h;
	}
};

class PhysicsServer : public Object {
  private:
	static const float	  CELL_SIZE;
	static PhysicsServer *_instance;

	static std::unordered_map<unsigned long, CollisionObject2D *>
		_collisionObjects;
	std::unordered_map<Vector2i, std::vector<CollisionObject2D *>, Vector2iHash>
		_hashMap;

	PhysicsServer(const std::string &instanceName = "");
	PhysicsServer(const PhysicsServer &) = delete;
	PhysicsServer &operator=(const PhysicsServer &) = delete;
	~PhysicsServer();

	inline Vector2i
	hashFunction(const raylib::Vector2 &position) const noexcept;

  public:
	static void addCollisionObject(CollisionObject2D *collisionObject) noexcept;
	static void
	removeCollisionObject(CollisionObject2D *collisionObject) noexcept;

	static PhysicsServer &getInstance() noexcept;
	static void			  deleteInstance() noexcept;

	// Core API
	void rebuild();
	void getCollisions(CollisionObject2D				&collisionObject,
					   std::vector<CollisionObject2D *> &out) const;

	// Legacy
	std::vector<CollisionObject2D *>
		 getCollisions(CollisionObject2D &physicsBody) const;
	void clear() noexcept;

	virtual const std::string &getClassName() const noexcept;
};

#endif // PHYSICSSERVER_HPP
