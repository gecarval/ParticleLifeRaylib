#include "RenderServer.hpp"
#include "../node/canvas_item/CanvasItem.hpp"

std::unordered_map<unsigned long, CanvasItem *> RenderServer::_canvasItems;
RenderServer *RenderServer::_instance = nullptr;

RenderServer::RenderServer(const std::string &instanceName)
	: Object(instanceName) {
}

RenderServer::~RenderServer() {
}

RenderServer &RenderServer::getInstance() noexcept {
	if (_instance == nullptr) {
		_instance = new RenderServer("RenderServer");
	}
	return *_instance;
}

void RenderServer::destroyInstance() noexcept {
	delete _instance;
	_instance = nullptr;
}

void RenderServer::render(const raylib::Window &window) noexcept {
	for (const auto &pair : _canvasItems) {
		CanvasItem *canvasItem = pair.second;
		if (canvasItem->isVisible()) {
			canvasItem->draw(window);
		}
		if (canvasItem->isVisibleDebug()) {
			canvasItem->drawDebug(window);
		}
	}
}

void RenderServer::render(const raylib::Window	 &window,
						  const raylib::Camera2D &camera) noexcept {
	for (const auto &pair : _canvasItems) {
		CanvasItem *canvasItem = pair.second;
		if (canvasItem->isVisible()) {
			canvasItem->draw(window, camera);
		}
		if (canvasItem->isVisibleDebug()) {
			canvasItem->drawDebug(window, camera);
		}
	}
}

void RenderServer::addCanvasItem(CanvasItem *canvasItem) noexcept {
	_canvasItems[canvasItem->getInstanceID()] = canvasItem;
}

void RenderServer::removeCanvasItem(CanvasItem *canvasItem) noexcept {
	_canvasItems.erase(canvasItem->getInstanceID());
}

const std::string &RenderServer::getClassName() const noexcept {
	static const std::string className("RenderServer");
	return className;
}
