#include "Control.hpp"

Control::Control(const std::string &instanceName) : CanvasItem(instanceName) {
}

Control::Control(const Control &other) : CanvasItem(other) {
}

Control &Control::operator=(const Control &other) {
	if (this != &other) {
		CanvasItem::operator=(other);
		// Copy derived class members here
	}
	return *this;
}

Control::~Control() {
}

const std::string &Control::getClassName() const noexcept {
	static const std::string className("Control");
	return className;
}
