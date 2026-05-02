#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "../../../../../include/raylib-cpp.hpp"
#include "../Node2D.hpp"

class Particle : public Node2D {
  private:
	static const float _defaultSize;

	raylib::Color _col;
	float		  _rad;

  public:
	Particle(const std::string	   &instanceName = "",
			 const raylib::Vector2 &pos = raylib::Vector2(),
			 const raylib::Color   &col = raylib::Color::White());
	Particle(const Particle &other);
	Particle &operator=(const Particle &other);
	virtual ~Particle();

	// methods
	virtual void updatePhysics() noexcept override;
	virtual void draw() const noexcept override;
	virtual void drawDebug() const noexcept override;

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
	const raylib::Color &getCol() const noexcept;
	raylib::Color		&getCol() noexcept;
	void				 setCol(const raylib::Color &newCol) noexcept;
	unsigned int		 getRadius() const noexcept;
	void				 setRadius(unsigned int newRad) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // PARTICLE_HPP
