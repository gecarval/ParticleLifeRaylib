#ifndef PHYSICSBODY2D_HPP
#define PHYSICSBODY2D_HPP

#include "../CollisionObject2D.hpp"

class PhysicsBody2D : public CollisionObject2D {
  protected:
	raylib::Vector2 _linearVel;
	raylib::Vector2 _angularVel;
	raylib::Vector2 _linearAcc;
	raylib::Vector2 _angularAcc;
	float			_mass;
	float			_friction;
	float			_restitution;
	bool			_is_static;
	bool			_lock_rotation;

  public:
	PhysicsBody2D(const std::string &instanceName = "");
	PhysicsBody2D(const PhysicsBody2D &other);
	PhysicsBody2D &operator=(const PhysicsBody2D &other);
	virtual ~PhysicsBody2D();

	// override methods
	virtual void ready() noexcept override;
	virtual void update() noexcept override;
	virtual void updatePhysics() noexcept override;
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
	void collideWith(PhysicsBody2D &other,
					 const float	restitution = 1.0f) noexcept;
	void collissionCircleCircle(PhysicsBody2D &other,
								const float	   restitution = 1.0f) noexcept;
	void collissionCircleRectangle(PhysicsBody2D &other,
								   const float	  restitution = 1.0f) noexcept;
	void collissionRectangleRectangle(PhysicsBody2D &other,
									  const float restitution = 1.0f) noexcept;

	// getters and setters
	raylib::Vector2 getLinearVel() const noexcept;
	void			setLinearVel(const raylib::Vector2 &newLinearVel) noexcept;
	raylib::Vector2 getAngularVel() const noexcept;
	void setAngularVel(const raylib::Vector2 &newAngularVel) noexcept;

	raylib::Vector2 getLinearAcc() const noexcept;
	void			setLinearAcc(const raylib::Vector2 &newLinearAcc) noexcept;
	raylib::Vector2 getAngularAcc() const noexcept;
	void setAngularAcc(const raylib::Vector2 &newAngularAcc) noexcept;

	float getMass() const noexcept;
	void  setMass(const float newMass) noexcept;
	float getFriction() const noexcept;
	void  setFriction(const float newFriction) noexcept;
	float getRestitution() const noexcept;
	void  setRestitution(const float newRestitution) noexcept;
	bool  isStatic() const noexcept;
	void  setStatic(const bool newIsStatic) noexcept;
	bool  isRotationLocked() const noexcept;
	void  setLockRotation(const bool newLockRotation) noexcept;

	const virtual std::string &getClassName() const noexcept override;
};

#endif // PHYSICSBODY2D_HPP
