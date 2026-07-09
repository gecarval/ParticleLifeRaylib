#ifndef PHYSICSSERVER_HPP
#define PHYSICSSERVER_HPP

#include "../Object.hpp"
#include "../Vector2i.hpp"
#include <unordered_map>

class CollisionObject2D;

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
