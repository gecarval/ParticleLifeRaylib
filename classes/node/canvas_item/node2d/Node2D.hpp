#ifndef NODE2D_HPP
#define NODE2D_HPP

#include "../CanvasItem.hpp"

class Node2D : public CanvasItem {
  protected:
	raylib::Vector2 _pos;
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
	virtual void		   setPos(const raylib::Vector2 &newPos) noexcept;
	float				   getRotation() const noexcept;
	virtual void		   addRotation(const float newRot) noexcept;
	virtual void		   resetRotation() noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // NODE2D_HPP
