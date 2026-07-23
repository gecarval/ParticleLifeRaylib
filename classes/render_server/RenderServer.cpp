#include "RenderServer.hpp"
#include "../node/canvas_item/CanvasItem.hpp"
#include "../node/canvas_item/node2d/Node2D.hpp"
#include "../node/canvas_item/node2d/sprite2d/Sprite2D.hpp"

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

void RenderServer::deleteInstance() noexcept {
	delete _instance;
	_instance = nullptr;
}

void RenderServer::render(const raylib::Window &window) noexcept {
	for (const auto &pair : _canvasItems) {
		bool		inView = false;
		bool		isSprite = false;
		CanvasItem *canvasItem = pair.second;
		Sprite2D   *sprite = dynamic_cast<Sprite2D *>(canvasItem);
		if (sprite) {
			isSprite = true;
			raylib::Vector2 pos = sprite->getPos();
			const Shape2D  *shape = sprite->getShape();
			if (shape && canvasItem->isInView(window, *shape, pos)) {
				inView = true;
			}
		}
		if (!isSprite) {
			Node2D *node = dynamic_cast<Node2D *>(canvasItem);
			if (node) {
				raylib::Vector2 pos = node->getPos();
				if (canvasItem->isInView(window, pos)) {
					inView = true;
				}
			}
		}
		if (!inView) {
			continue;
		}
		if (canvasItem->isVisible()) {
			canvasItem->draw();
		}
		if (canvasItem->isVisibleDebug()) {
			canvasItem->drawDebug();
		}
	}
}

void RenderServer::render(const raylib::Window	 &window,
						  const raylib::Camera2D &camera) noexcept {
	for (const auto &pair : _canvasItems) {
		bool		inView = false;
		bool		isSprite = false;
		CanvasItem *canvasItem = pair.second;
		Sprite2D   *sprite = dynamic_cast<Sprite2D *>(canvasItem);
		if (sprite) {
			isSprite = true;
			raylib::Vector2 pos = sprite->getPos();
			const Shape2D  *shape = sprite->getShape();
			if (shape && canvasItem->isInView(window, camera, *shape, pos)) {
				inView = true;
			}
		}
		if (!isSprite) {
			Node2D *node = dynamic_cast<Node2D *>(canvasItem);
			if (node) {
				raylib::Vector2 pos = node->getPos();
				if (canvasItem->isInView(window, camera, pos)) {
					inView = true;
				}
			}
		}
		if (!inView) {
			continue;
		}
		if (canvasItem->isVisible()) {
			canvasItem->draw();
		}
		if (canvasItem->isVisibleDebug()) {
			canvasItem->drawDebug();
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
