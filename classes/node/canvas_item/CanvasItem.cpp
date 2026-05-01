#include "CanvasItem.hpp"

CanvasItem::CanvasItem(const std::string &instanceName)
	: Object(instanceName), Node(instanceName), _visible(true),
	  _visibleDebug(false), _layer(0) {
}

CanvasItem::CanvasItem(const CanvasItem &other)
	: Object(other), Node(other), _visible(other._visible),
	  _visibleDebug(other._visibleDebug), _layer(other._layer) {
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

CanvasItem::~CanvasItem() {
}

bool CanvasItem::isVisible() const noexcept {
	return _visible;
}

void CanvasItem::setVisible(bool visible) noexcept {
	_visible = visible;
}

bool CanvasItem::isVisibleDebug() const noexcept {
	return _visibleDebug;
}

void CanvasItem::setVisibleDebug(bool visibleDebug) noexcept {
	_visibleDebug = visibleDebug;
}

int CanvasItem::getLayer() const noexcept {
	return _layer;
}

void CanvasItem::setLayer(int layer) noexcept {
	_layer = layer;
}

const std::string &CanvasItem::getClassName() const noexcept {
	static const std::string className("CanvasItem");
	return className;
}
