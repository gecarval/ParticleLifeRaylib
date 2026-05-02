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
	void setShape(const float width, const float height) noexcept;
	void setShape(const raylib::Vector2 &size) noexcept;
	void setShape(const float radius) noexcept;

	// getters and getters
	const Shape2D *getShape() const noexcept;
	Shape2D		  *getShape() noexcept;
	void		   setShape(Shape2D *newShape) noexcept;
	bool		   isEnabled() const noexcept;
	void		   setEnabled(const bool newEnabled) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // COLLISIONSHAPE2D_HPP
