#ifndef RENDERSERVER_HPP
#define RENDERSERVER_HPP

#include "../../include/raylib-cpp.hpp"
#include "../Object.hpp"
#include <unordered_map>

class CanvasItem;

class RenderServer : public Object {
  private:
	static std::unordered_map<unsigned long, CanvasItem *> _canvasItems;
	static RenderServer									  *_instance;

	RenderServer(const std::string &instanceName = "");
	RenderServer(const RenderServer &other) = delete;
	RenderServer &operator=(const RenderServer &other) = delete;
	virtual ~RenderServer();

  public:
	static RenderServer &getInstance() noexcept;
	static void			 destroyInstance() noexcept;

	void		render(const raylib::Window &window) noexcept;
	void		render(const raylib::Window	  &window,
					   const raylib::Camera2D &camera) noexcept;
	static void addCanvasItem(CanvasItem *canvasItem) noexcept;
	static void removeCanvasItem(CanvasItem *canvasItem) noexcept;

	virtual const std::string &getClassName() const noexcept;
};

#endif // RENDERSERVER_HPP
