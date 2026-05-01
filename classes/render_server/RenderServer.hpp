#ifndef RENDERSERVER_HPP
#define RENDERSERVER_HPP

#include "../Object.hpp"

class RenderServer : public Object {
  private:
  public:
	RenderServer();
	RenderServer(const RenderServer &other);
	RenderServer &operator=(const RenderServer &other);
	virtual ~RenderServer();

	virtual const std::string &getClassName() const noexcept;
};

#endif // RENDERSERVER_HPP
