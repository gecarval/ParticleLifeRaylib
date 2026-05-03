#include "CanvasItem.hpp"
#include "../../render_server/RenderServer.hpp"
#include "../../shape2d/rectangle_shape2d/RectangleShape2D.hpp"

CanvasItem::CanvasItem(const std::string &instanceName)
	: Node(instanceName), _visible(true), _visibleDebug(false), _layer(0) {
	RenderServer::addCanvasItem(this);
}

CanvasItem::CanvasItem(const CanvasItem &other)
	: Node(other), _visible(other._visible), _visibleDebug(other._visibleDebug),
	  _layer(other._layer) {
	RenderServer::addCanvasItem(this);
}

CanvasItem &CanvasItem::operator=(const CanvasItem &other) {
	if (this != &other) {
		Node::operator=(other);
		_visible = other._visible;
		_visibleDebug = other._visibleDebug;
		_layer = other._layer;
	}
	return *this;
}

bool CanvasItem::isInView(const raylib::Window	&window,
						  const raylib::Vector2 &pos) const noexcept {
	const raylib::Vector2	size(window.GetWidth(), window.GetHeight());
	const raylib::Rectangle screenSpace(raylib::Vector2::Zero(), size);
	if (screenSpace.CheckCollision(pos)) {
		return (true);
	}
	return (false);
}

bool CanvasItem::isInView(const raylib::Window	 &window,
						  const raylib::Camera2D &camera,
						  const raylib::Vector2	 &pos) const noexcept {
	const raylib::Vector2 posScreen(
		camera.GetTarget().x - (window.GetWidth() / 2.0f) / camera.GetZoom(),
		camera.GetTarget().y - (window.GetHeight() / 2.0f) / camera.GetZoom());
	const raylib::Vector2	size(window.GetWidth() / camera.GetZoom(),
								 window.GetHeight() / camera.GetZoom());
	const raylib::Rectangle screenSpace(posScreen, size);
	if (screenSpace.CheckCollision(pos)) {
		return (true);
	}
	return (false);
}

bool CanvasItem::isInView(const raylib::Window &window, const Shape2D &shape,
						  const raylib::Vector2 &shapePos) const noexcept {
	const raylib::Vector2  pos;
	const raylib::Vector2  size(window.GetWidth(), window.GetHeight());
	const RectangleShape2D screenSpace(size);
	if (screenSpace.collides(pos, shape, shapePos)) {
		return (true);
	}
	return (false);
}

bool CanvasItem::isInView(const raylib::Window	 &window,
						  const raylib::Camera2D &camera, const Shape2D &shape,
						  const raylib::Vector2 &shapePos) const noexcept {
	const raylib::Vector2 pos(
		camera.GetTarget().x - (window.GetWidth() / 2.0f) / camera.GetZoom(),
		camera.GetTarget().y - (window.GetHeight() / 2.0f) / camera.GetZoom());
	const raylib::Vector2  size(window.GetWidth() / camera.GetZoom(),
								window.GetHeight() / camera.GetZoom());
	const RectangleShape2D screenSpace(size);
	if (screenSpace.collides(pos, shape, shapePos)) {
		return (true);
	}
	return (false);
}

CanvasItem::~CanvasItem() {
	RenderServer::removeCanvasItem(this);
}

bool CanvasItem::isVisible() const noexcept {
	return _visible;
}

void CanvasItem::setVisible(const bool visible) noexcept {
	_visible = visible;
}

bool CanvasItem::isVisibleDebug() const noexcept {
	return _visibleDebug;
}

void CanvasItem::setVisibleDebug(const bool visibleDebug) noexcept {
	_visibleDebug = visibleDebug;
}

int CanvasItem::getLayer() const noexcept {
	return _layer;
}

void CanvasItem::setLayer(const int layer) noexcept {
	_layer = layer;
}

const std::string &CanvasItem::getClassName() const noexcept {
	static const std::string className("CanvasItem");
	return className;
}
