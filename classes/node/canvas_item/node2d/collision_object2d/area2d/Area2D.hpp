#ifndef AREA2D_HPP
#define AREA2D_HPP

#include "../CollisionObject2D.hpp"

class Area2D : public CollisionObject2D {
  private:
  public:
	Area2D();
	Area2D(const Area2D &other);
	Area2D &operator=(const Area2D &other);
	virtual ~Area2D();
};

#endif // AREA2D_HPP
