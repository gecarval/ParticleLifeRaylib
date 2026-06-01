#ifndef COLLISIONOBJECT2D_HPP
#define COLLISIONOBJECT2D_HPP

#include "../Node2D.hpp"

class CollisionObject2D : public Node2D {
  protected:
	int	 _collisionLayer;
	int	 _collisionMask;
	bool _enabled;

  public:
	CollisionObject2D(const std::string &instanceName = "");
	CollisionObject2D(const CollisionObject2D &other);
	CollisionObject2D &operator=(const CollisionObject2D &other);
	virtual ~CollisionObject2D();

	// getters and setters
	int				   getCollisionLayer() const noexcept;
	CollisionObject2D &setCollisionLayer(int newLayer) noexcept;
	int				   getCollisionMask() const noexcept;
	CollisionObject2D &setCollisionMask(int newMask) noexcept;
	bool			   isEnabled() const noexcept;
	CollisionObject2D &setEnabled(const bool newEnabled) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // COLLISIONOBJECT2D_HPP
