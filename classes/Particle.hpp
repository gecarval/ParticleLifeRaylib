#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../include/raylib-cpp.hpp"

class Particle {
  private:
	static const float _defaultSize;

	// physics
	raylib::Vector2 _pos;
	raylib::Vector2 _vel;
	raylib::Vector2 _acc;
	raylib::Color	_col;
	float			_rad;

  public:
	Particle(const raylib::Vector2 &pos = raylib::Vector2());
	Particle(const raylib::Color   &col,
			 const raylib::Vector2 &pos = raylib::Vector2());
	Particle(const Particle &other);
	Particle &operator=(const Particle &other);
	virtual ~Particle();

	// methods
	void update(void) noexcept;
	void draw(void) const noexcept;
	void debugDraw(void) const noexcept;

	// movement methods
	void applyForce(const raylib::Vector2 &force) noexcept;
	void moveTowards(const raylib::Vector2 &target,
					 const float			strength = 1.0f) noexcept;
	void moveAwayFrom(const raylib::Vector2 &target,
					  const float			 strength = 1.0f) noexcept;

	// physics methods
	void applyGravity(const float strength = 9.81f) noexcept;
	void applyNewtonianGravity(const raylib::Vector2 &target,
							   const float strength = 9.81f) noexcept;
	void applyFriction(const float strength = 0.1f) noexcept;
	void collideWith(Particle &other, const float restitution = 1.0f) noexcept;

	// getters and setters
	const raylib::Vector2 &getPos(void) const noexcept;
	raylib::Vector2		  &getPos(void) noexcept;
	void				   setPos(const raylib::Vector2 &newPos) noexcept;
	const raylib::Vector2 &getVel(void) const noexcept;
	raylib::Vector2		  &getVel(void) noexcept;
	void				   setVel(const raylib::Vector2 &newVel) noexcept;
	const raylib::Vector2 &getAcc(void) const noexcept;
	raylib::Vector2		  &getAcc(void) noexcept;
	void				   setAcc(const raylib::Vector2 &newAcc) noexcept;
	const raylib::Color	  &getCol(void) const noexcept;
	raylib::Color		  &getCol(void) noexcept;
	void				   setCol(const raylib::Color &newCol) noexcept;
	unsigned int		   getRadius(void) const noexcept;
	void				   setRadius(unsigned int newRad) noexcept;
};

#endif // PARTICLE_HPP
