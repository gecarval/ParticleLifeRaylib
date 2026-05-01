#include "Node.hpp"

Node::Node(const std::string &instanceName) noexcept : Object(instanceName) {
}

Node::~Node() {
	for (unsigned long i = 0; i < _children.size(); i++) {
		delete _children[i];
	}
}

Node::Node(const Node &other) noexcept
	: Object(other), _children(other._children) {
}

Node &Node::operator=(const Node &other) noexcept {
	if (this != &other) {
		Object::operator=(other);
		_children = other._children;
	}
	return (*this);
}

std::vector<Object *> Node::getChildren() const noexcept {
	return (_children);
}

std::vector<Object *>
Node::findChild(const std::string &instanceName) noexcept {
	std::vector<Object *> children;
	unsigned long		  i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

std::vector<Object *> Node::findChild(const unsigned long instanceID) noexcept {
	std::vector<Object *> children;
	unsigned long		  i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

std::vector<const Object *>
Node::findChild(const std::string &instanceName) const noexcept {
	std::vector<const Object *> children;
	unsigned long				i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

std::vector<const Object *>
Node::findChild(const unsigned long instanceID) const noexcept {
	std::vector<const Object *> children;
	unsigned long				i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

std::vector<Object *> Node::findClass(const std::string &className) noexcept {
	std::vector<Object *> children;
	unsigned long		  i = 0;
	while (i < _children.size()) {
		if (className == _children[i]->getClassName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

std::vector<const Object *>
Node::findClass(const std::string &className) const noexcept {
	std::vector<const Object *> children;
	unsigned long				i = 0;
	while (i < _children.size()) {
		if (className == _children[i]->getClassName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return (children);
}

void Node::insertChild(Object &child, const Node::iterator &it) noexcept {
	if (it < _children.begin() || it >= _children.end()) {
		return;
	}
	_children.insert(it, &child);
}

void Node::pushFrontChild(Object &child) noexcept {
	_children.insert(_children.begin(), &child);
}

void Node::pushBackChild(Object &child) noexcept {
	_children.push_back(&child);
}

Object *Node::backChild() noexcept {
	return (_children.back());
}

Object *Node::frontChild() noexcept {
	return (_children.front());
}

void Node::popFrontChild() noexcept {
	_children.erase(_children.begin());
}

void Node::popBackChild() noexcept {
	_children.pop_back();
}

void Node::deleteChild(const Node::iterator &it) noexcept {
	if (it < _children.begin() || it >= _children.end()) {
		return;
	}
	delete *it;
	_children.erase(it);
}

void Node::deleteChild(const std::string &instanceName) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			delete _children[i];
			_children.erase(_children.begin() + i);
			continue;
		}
		i++;
	}
}

void Node::deleteChild(const unsigned long instanceID) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			delete _children[i];
			_children.erase(_children.begin() + i);
			break;
		}
		i++;
	}
}

void Node::eraseChild(const Node::iterator &it) noexcept {
	if (it < _children.begin() || it >= _children.end()) {
		return;
	}
	_children.erase(it);
}

void Node::eraseChild(const std::string &instanceName) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			_children.erase(_children.begin() + i);
			continue;
		}
		i++;
	}
}

void Node::eraseChild(const unsigned long instanceID) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			_children.erase(_children.begin() + i);
			break;
		}
		i++;
	}
}

const std::string &Node::getClassName() const noexcept {
	static const std::string className("Node");
	return (className);
}

std::ostream &operator<<(std::ostream &out, const Node &other) {
	const Object &otherObject = other;
	out << otherObject;
	out << "; children";
	out << other.getChildren();
	return (out);
}
