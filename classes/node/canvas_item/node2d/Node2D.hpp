#ifndef NODE2D_HPP
#define NODE2D_HPP

#include "../CanvasItem.hpp"

class Node2D : public CanvasItem {
  protected:
	raylib::Vector2 _pos;
	raylib::Vector2 _vel;
	raylib::Vector2 _acc;
	float			_rot;

  public:
	Node2D(const std::string &instanceName = "");
	Node2D(const Node2D &other);
	Node2D &operator=(const Node2D &other);
	virtual ~Node2D();

	// draw calls
	virtual void draw() const noexcept override;
	virtual void drawDebug() const noexcept override;

	// getters and setters
	const raylib::Vector2 &getPos() const noexcept;
	raylib::Vector2		  &getPos() noexcept;
	void				   setPos(const raylib::Vector2 &newPos) noexcept;
	const raylib::Vector2 &getVel() const noexcept;
	raylib::Vector2		  &getVel() noexcept;
	void				   setVel(const raylib::Vector2 &newVel) noexcept;
	const raylib::Vector2 &getAcc() const noexcept;
	raylib::Vector2		  &getAcc() noexcept;
	void				   setAcc(const raylib::Vector2 &newAcc) noexcept;
	float				   getRotation() const noexcept;
	void				   setRotation(const float newRot) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // NODE2D_HPP
