#include "RenderServer.hpp"

RenderServer::RenderServer() {
}

RenderServer::RenderServer(const RenderServer &other) : Object(other) {
}

RenderServer &RenderServer::operator=(const RenderServer &other) {
	if (this != &other) {
		Object::operator=(other);
		// Copy derived class members here
	}
	return *this;
}

RenderServer::~RenderServer() {
}
