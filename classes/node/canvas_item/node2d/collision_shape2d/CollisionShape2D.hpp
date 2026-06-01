#ifndef COLLISIONSHAPE2D_HPP
#define COLLISIONSHAPE2D_HPP

#include "../../../../shape2d/Shape2D.hpp"
#include "../Node2D.hpp"

class CollisionShape2D : public Node2D {
  protected:
	Shape2D *_shape;
	bool	 _enabled;

  public:
	CollisionShape2D(const std::string &instanceName = "");
	CollisionShape2D(const raylib::Rectangle &rect,
					 const std::string		 &instanceName = "");
	CollisionShape2D(const raylib::Vector2 &point, const float radius,
					 const std::string &instanceName = "");
	CollisionShape2D(const CollisionShape2D &other);
	CollisionShape2D &operator=(const CollisionShape2D &other);
	virtual ~CollisionShape2D();

	// draw calls
	virtual void drawDebug() const noexcept override;

	// utility functions
	CollisionShape2D &setShape(const float width, const float height) noexcept;
	CollisionShape2D &setShape(const raylib::Vector2 &size) noexcept;
	CollisionShape2D &setShape(const float radius) noexcept;

	// getters and getters
	CollisionShape2D &setParentPos(const raylib::Vector2 &newPos) noexcept;
	CollisionShape2D &setParentRotation(const float newRotation) noexcept;
	raylib::Vector2	  getParentPos() const noexcept;
	float			  getParentRotation() const noexcept;
	const Shape2D	 *getShape() const noexcept;
	Shape2D			 *getShape() noexcept;
	CollisionShape2D &setShape(Shape2D *newShape) noexcept;
	bool			  isEnabled() const noexcept;
	CollisionShape2D &setEnabled(const bool newEnabled) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // COLLISIONSHAPE2D_HPP
