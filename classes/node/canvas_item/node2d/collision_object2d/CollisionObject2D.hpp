#ifndef COLLISIONOBJECT2D_HPP
#define COLLISIONOBJECT2D_HPP

#include "../Node2D.hpp"
#include "../collision_shape2d/CollisionShape2D.hpp"

class CollisionObject2D : public Node2D {
  protected:
	int _collisionLayer;
	int _collisionMask;

  public:
	CollisionObject2D(const std::string &instanceName = "");
	CollisionObject2D(const CollisionObject2D &other);
	CollisionObject2D &operator=(const CollisionObject2D &other);
	virtual ~CollisionObject2D();

	// getters and setters
	void setCollisionShape(const CollisionShape2D &newShape) noexcept;
	int	 getCollisionLayer() const noexcept;
	void setCollisionLayer(int newLayer) noexcept;
	int	 getCollisionMask() const noexcept;
	void setCollisionMask(int newMask) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // COLLISIONOBJECT2D_HPP
