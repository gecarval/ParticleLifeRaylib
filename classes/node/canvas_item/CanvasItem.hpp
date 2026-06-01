#ifndef CANVASITEM_HPP
#define CANVASITEM_HPP

#include "../../../include/raylib-cpp.hpp"
#include "../../shape2d/Shape2D.hpp"
#include "../Node.hpp"

class CanvasItem : public Node {
  private:
	bool _visible;
	bool _visibleDebug;
	int	 _layer;

  public:
	CanvasItem(const std::string &instanceName = "");
	CanvasItem(const CanvasItem &other);
	CanvasItem &operator=(const CanvasItem &other);
	virtual ~CanvasItem();

	// draw method to be implemented by derived classes
	bool isInView(const raylib::Window	&window,
				  const raylib::Vector2 &pos) const noexcept;
	bool isInView(const raylib::Window &window, const raylib::Camera2D &camera,
				  const raylib::Vector2 &pos) const noexcept;
	bool isInView(const raylib::Window &window, const Shape2D &shape,
				  const raylib::Vector2 &pos) const noexcept;
	bool isInView(const raylib::Window &window, const raylib::Camera2D &camera,
				  const Shape2D			&shape,
				  const raylib::Vector2 &pos) const noexcept;
	virtual void draw() const noexcept = 0;
	virtual void drawDebug() const noexcept = 0;

	// getters and setters
	bool		isVisible() const noexcept;
	CanvasItem &setVisible(const bool visible) noexcept;
	bool		isVisibleDebug() const noexcept;
	CanvasItem &setVisibleDebug(const bool visibleDebug) noexcept;
	int			getLayer() const noexcept;
	CanvasItem &setLayer(const int layer) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

#endif // CANVASITEM_HPP
