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

Object &Object::operator=(const Object &other) noexcept {
	if (this != &other) {
		_instanceName = other._instanceName;
	}
	return (*this);
}

unsigned long Object::getInstanceID() const noexcept {
	return (_instanceID);
}

const std::string &Object::getInstanceName() const noexcept {
	return (_instanceName);
}

std::string &Object::getInstanceName() noexcept {
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

const std::string &Object::getClassName() const noexcept {
	static const std::string className("Object");
	return (className);
}

std::ostream &operator<<(std::ostream &out, const Object &other) {
	out << other.getClassName() << "#" << other.getInstanceID() << ": \""
		<< other.getInstanceName() << "\"";
	return (out);
}
