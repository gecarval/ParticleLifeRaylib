#ifndef CANVASITEM_HPP
#define CANVASITEM_HPP

#include "../../../include/raylib-cpp.hpp"
#include "../Node.hpp"

class CanvasItem : virtual public Node {
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
	virtual void draw(const raylib::Window &window) const noexcept = 0;
	virtual void draw(const raylib::Window	 &window,
					  const raylib::Camera2D &camera) const noexcept = 0;
	virtual void drawDebug(const raylib::Window &window) const noexcept = 0;
	virtual void drawDebug(const raylib::Window	  &window,
						   const raylib::Camera2D &camera) const noexcept = 0;

	// getters and setters
	bool isVisible() const noexcept;
	void setVisible(const bool visible) noexcept;
	bool isVisibleDebug() const noexcept;
	void setVisibleDebug(const bool visibleDebug) noexcept;
	int	 getLayer() const noexcept;
	void setLayer(const int layer) noexcept;

	virtual const std::string &getClassName() const noexcept;
};

#endif // CANVASITEM_HPP
