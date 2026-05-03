#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../../../sprite2d/Sprite2D.hpp"
#include "../PhysicsBody2D.hpp"

class Particle : public PhysicsBody2D {
  private:
	static const float _defaultSize;

  public:
	Particle(const std::string	   &instanceName = "",
			 const raylib::Vector2 &pos = raylib::Vector2(),
			 const raylib::Color   &col = raylib::Color::White());
	Particle(const Particle &other);
	Particle &operator=(const Particle &other);
	virtual ~Particle();

	// methods
	virtual void draw() const noexcept override;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // PARTICLE_HPP
