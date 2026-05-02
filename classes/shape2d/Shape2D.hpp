#ifndef SHAPE2D_HPP
#define SHAPE2D_HPP

#include "../../include/raylib-cpp.hpp"
#include "../Object.hpp"

class Shape2D : public Object {
  public:
	Shape2D(const std::string &name = "Shape2D");
	Shape2D(const Shape2D &other);
	Shape2D &operator=(const Shape2D &other);
	virtual ~Shape2D();

	virtual void drawDebug(const raylib::Vector2 &pos) const noexcept = 0;
	virtual bool collides(const raylib::Vector2 &thisPos, const Shape2D &other,
						  const raylib::Vector2 &otherPos) const noexcept = 0;
	virtual bool
	collidesOnMotion(const raylib::Vector2 &thisPos,
					 const raylib::Vector2 &thisVel, const Shape2D &other,
					 const raylib::Vector2 &otherPos,
					 const raylib::Vector2 &otherVel) const noexcept = 0;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // SHAPE2D_HPP
