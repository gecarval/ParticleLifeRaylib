#ifndef CIRCLESHAPE2D_HPP
#define CIRCLESHAPE2D_HPP

#include "../Shape2D.hpp"

class CircleShape2D : public Shape2D {
  protected:
	raylib::Vector2 _pos;
	float			_radius;

  public:
	CircleShape2D(const float radius = 1.0f);
	CircleShape2D(const float x = 0, const float y = 0,
				  const float radius = 1.0f);
	CircleShape2D(const raylib::Vector2 &pos, const float radius = 1.0f);
	CircleShape2D(const CircleShape2D &other);
	CircleShape2D &operator=(const CircleShape2D &other);
	virtual ~CircleShape2D();

	void			setPosition(const raylib::Vector2 &pos) noexcept;
	raylib::Vector2 getPosition() const noexcept;
	void			setRadius(const float radius) noexcept;
	float			getRadius() const noexcept;

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

#endif // CIRCLESHAPE2D_HPP
