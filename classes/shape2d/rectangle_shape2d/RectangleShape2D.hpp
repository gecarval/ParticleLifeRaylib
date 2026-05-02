#ifndef RECTANGLESHAPE2D_HPP
#define RECTANGLESHAPE2D_HPP

#include "../Shape2D.hpp"

class RectangleShape2D : public Shape2D {
  protected:
	raylib::Vector2 _size;

  public:
	RectangleShape2D(const float width = 1.0f, const float height = 1.0f);
	RectangleShape2D(const raylib::Vector2 &size = raylib::Vector2::One());
	RectangleShape2D(const RectangleShape2D &other);
	RectangleShape2D &operator=(const RectangleShape2D &other);
	virtual ~RectangleShape2D();

	raylib::Vector2 getSize() const noexcept;
	void			setSize(const raylib::Vector2 &size) noexcept;

	virtual void drawDebug(const raylib::Vector2 &pos) const noexcept override;
	virtual bool
	collides(const raylib::Vector2 &thisPos, const Shape2D &other,
			 const raylib::Vector2 &otherPos) const noexcept override;
	virtual bool
	collidesOnMotion(const raylib::Vector2 &thisPos,
					 const raylib::Vector2 &thisVel, const Shape2D &other,
					 const raylib::Vector2 &otherPos,
					 const raylib::Vector2 &otherVel) const noexcept override;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // RECTANGLESHAPE2D_HPP
