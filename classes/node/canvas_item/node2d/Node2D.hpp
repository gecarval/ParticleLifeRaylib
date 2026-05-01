#ifndef NODE2D_HPP
#define NODE2D_HPP

#include "../CanvasItem.hpp"

class Node2D : virtual public CanvasItem {
  private:
  public:
	Node2D(const std::string &instanceName = "");
	Node2D(const Node2D &other);
	Node2D &operator=(const Node2D &other);
	virtual ~Node2D();

	virtual const std::string &getClassName() const noexcept;
};

#endif // NODE2D_HPP
