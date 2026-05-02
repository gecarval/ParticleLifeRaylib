#ifndef RECTANGLESHAPE2D_HPP
#define RECTANGLESHAPE2D_HPP

#include "../Shape2D.hpp"

class RectangleShape2D : public Shape2D {
  protected:
	raylib::Rectangle _shape;

  public:
	RectangleShape2D(const float x = 0, const float y = 0,
					 const float width = 0, const float height = 0);
	RectangleShape2D(const raylib::Vector2 &pos = raylib::Vector2(0, 0),
					 const raylib::Vector2 &size = raylib::Vector2(0, 0));
	RectangleShape2D(const raylib::Rectangle &rect = raylib::Rectangle());
	RectangleShape2D(const RectangleShape2D &other);
	RectangleShape2D &operator=(const RectangleShape2D &other);
	virtual ~RectangleShape2D();

	void					 setShape(const raylib::Rectangle &rect) noexcept;
	raylib::Rectangle		&getShape() noexcept;
	const raylib::Rectangle &getShape() const noexcept;

	virtual void drawDebug() const noexcept override;
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
