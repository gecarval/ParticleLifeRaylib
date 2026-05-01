#include "Object.hpp"

unsigned long Object::_instanceIDCounter = 0;

Object::Object(const std::string &instanceName)
	: _instanceID(_instanceIDCounter++), _instanceName(instanceName) {
}

Object::~Object() {
}

Object::Object(const Object &other)
	: _instanceID(_instanceIDCounter++), _instanceName(other._instanceName) {
}

Object &Object::operator=(const Object &other) {
	if (this != &other) {
		_instanceName = other._instanceName;
	}
	return (*this);
}

unsigned long Object::getInstanceID(void) const noexcept {
	return (_instanceID);
}

const std::string &Object::getInstanceName(void) const noexcept {
	return (_instanceName);
}

std::string &Object::getInstanceName(void) noexcept {
	return (_instanceName);
}

void Object::setInstanceName(const std::string &instanceName) noexcept {
	_instanceName = instanceName;
}

bool Object::operator==(const Object &other) const noexcept {
	return (_instanceID == other._instanceID);
}

bool Object::operator!=(const Object &other) const noexcept {
	return (_instanceID != other._instanceID);
}

const std::string &Object::getClassName(void) const noexcept {
	static const std::string className("Object");
	return (className);
}
