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
	virtual void updatePhysics() noexcept override;
	virtual void drawDebug() const noexcept override;

	// movement methods
	PhysicsBody2D &applyForce(const raylib::Vector2 &force) noexcept;
	PhysicsBody2D &moveTowards(const raylib::Vector2 &target,
							   const float			  strength = 1.0f) noexcept;
	PhysicsBody2D &moveAwayFrom(const raylib::Vector2 &target,
								const float strength = 1.0f) noexcept;

	// physics methods
	PhysicsBody2D &applyGravity(const float strength = 9.81f) noexcept;
	PhysicsBody2D &applyNewtonianGravity(const raylib::Vector2 &target,
										 const float strength = 9.81f) noexcept;
	PhysicsBody2D &applyFriction(const float strength = 0.1f) noexcept;
	PhysicsBody2D &resolveContact(PhysicsBody2D			&other,
								  const raylib::Vector2 &normal,
								  const raylib::Vector2 &contactPt,
								  const float overlap, const float inertiaA,
								  const float inertiaB,
								  const float restitution) noexcept;
	PhysicsBody2D &collideWith(PhysicsBody2D &other,
							   const float	  restitution = 1.0f) noexcept;
	PhysicsBody2D &
	collissionCircleCircle(PhysicsBody2D &other,
						   const float	  restitution = 1.0f) noexcept;
	PhysicsBody2D &
	collissionCircleRectangle(PhysicsBody2D &other,
							  const float	 restitution = 1.0f) noexcept;
	PhysicsBody2D &
	collissionRectangleRectangle(PhysicsBody2D &other,
								 const float	restitution = 1.0f) noexcept;
	PhysicsBody2D &
	collissionRectangleCircle(PhysicsBody2D &other,
							  const float	 restitution = 1.0f) noexcept;

	// getters and setters
	raylib::Vector2 getLinearVel() const noexcept;
	PhysicsBody2D  &setLinearVel(const raylib::Vector2 &newLinearVel) noexcept;
	raylib::Vector2 getAngularVel() const noexcept;
	PhysicsBody2D &setAngularVel(const raylib::Vector2 &newAngularVel) noexcept;

	raylib::Vector2 getLinearAcc() const noexcept;
	PhysicsBody2D  &setLinearAcc(const raylib::Vector2 &newLinearAcc) noexcept;
	raylib::Vector2 getAngularAcc() const noexcept;
	PhysicsBody2D &setAngularAcc(const raylib::Vector2 &newAngularAcc) noexcept;

	float		   getMass() const noexcept;
	PhysicsBody2D &setMass(const float newMass) noexcept;
	float		   getFriction() const noexcept;
	PhysicsBody2D &setFriction(const float newFriction) noexcept;
	float		   getRestitution() const noexcept;
	PhysicsBody2D &setRestitution(const float newRestitution) noexcept;
	bool		   isStatic() const noexcept;
	PhysicsBody2D &setStatic(const bool newIsStatic) noexcept;
	bool		   isRotationLocked() const noexcept;
	PhysicsBody2D &setLockRotation(const bool newLockRotation) noexcept;

	const virtual std::string &getClassName() const noexcept override;
};

#endif // PHYSICSBODY2D_HPP
