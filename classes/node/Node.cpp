#include "Node.hpp"

Node::Node(const std::string &instanceName) noexcept
	: Object(instanceName), _parent(nullptr) {
}

Node::~Node() {
	for (unsigned long i = 0; i < _children.size(); i++) {
		delete _children[i];
	}
}

Node::Node(const Node &other) noexcept
	: Object(other), _parent(other._parent), _children(other._children) {
}

Node &Node::operator=(const Node &other) noexcept {
	if (this != &other) {
		Object::operator=(other);
		_children = other._children;
		_parent = other._parent;
	}
	return *this;
}

Node *Node::getParent() const noexcept {
	return _parent;
}

Node &Node::setParent(Node *parent) noexcept {
	if (_parent != nullptr) {
		_parent->removeChild(this->getInstanceID());
	}
	_parent = parent;
	return *this;
}

std::vector<Node *> Node::getChildren() const noexcept {
	return _children;
}

void Node::ready() noexcept {
}

void Node::update() noexcept {
}

void Node::updatePhysics() noexcept {
}

std::vector<Node *> Node::findChild(const std::string &instanceName) noexcept {
	std::vector<Node *> children;
	unsigned long		i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

std::vector<Node *> Node::findChild(const unsigned long instanceID) noexcept {
	std::vector<Node *> children;
	unsigned long		i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

std::vector<const Node *>
Node::findChild(const std::string &instanceName) const noexcept {
	std::vector<const Node *> children;
	unsigned long			  i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

std::vector<const Node *>
Node::findChild(const unsigned long instanceID) const noexcept {
	std::vector<const Node *> children;
	unsigned long			  i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

std::vector<Node *> Node::findClass(const std::string &className) noexcept {
	std::vector<Node *> children;
	unsigned long		i = 0;
	while (i < _children.size()) {
		if (className == _children[i]->getClassName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

std::vector<const Node *>
Node::findClass(const std::string &className) const noexcept {
	std::vector<const Node *> children;
	unsigned long			  i = 0;
	while (i < _children.size()) {
		if (className == _children[i]->getClassName()) {
			children.push_back(_children[i]);
		}
		i++;
	}
	return children;
}

Node &Node::insertChild(Node &child, const Node::iterator &it) noexcept {
	if (child._parent == this) {
		return *this;
	}
	if (it < _children.begin() || it >= _children.end()) {
		return *this;
	}
	(*it)->setParent(this);
	_children.insert(it, &child);
	return *this;
}

Node &Node::pushFrontChild(Node &child) noexcept {
	if (child._parent == this) {
		return *this;
	}
	child.setParent(this);
	_children.insert(_children.begin(), &child);
	return *this;
}

Node &Node::pushBackChild(Node &child) noexcept {
	if (child._parent == this) {
		return *this;
	}
	child.setParent(this);
	_children.push_back(&child);
	return *this;
}

Node *Node::backChild() noexcept {
	return _children.back();
}

Node *Node::frontChild() noexcept {
	return _children.front();
}

Node &Node::popFrontChild() noexcept {
	_children.front()->_parent = nullptr;
	_children.erase(_children.begin());
	return *this;
}

Node &Node::popBackChild() noexcept {
	_children.back()->_parent = nullptr;
	_children.pop_back();
	return *this;
}

Node &Node::deleteChild(const Node::iterator &it) noexcept {
	if (it < _children.begin() || it >= _children.end()) {
		return *this;
	}
	delete *it;
	_children.erase(it);
	return *this;
}

Node &Node::deleteChild(const std::string &instanceName) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			delete _children[i];
			_children.erase(_children.begin() + i);
			continue;
		}
		i++;
	}
	return *this;
}

Node &Node::deleteChild(const unsigned long instanceID) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			delete _children[i];
			_children.erase(_children.begin() + i);
			break;
		}
		i++;
	}
	return *this;
}

Node &Node::removeChild(const Node::iterator &it) noexcept {
	if (it < _children.begin() || it >= _children.end()) {
		return *this;
	}
	(*it)->_parent = nullptr;
	_children.erase(it);
	return *this;
}

Node &Node::removeChild(const std::string &instanceName) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceName == _children[i]->getInstanceName()) {
			_children[i]->_parent = nullptr;
			_children.erase(_children.begin() + i);
			continue;
		}
		i++;
	}
	return *this;
}

Node &Node::removeChild(const unsigned long instanceID) noexcept {
	unsigned long i = 0;
	while (i < _children.size()) {
		if (instanceID == _children[i]->getInstanceID()) {
			_children[i]->_parent = nullptr;
			_children.erase(_children.begin() + i);
			break;
		}
		i++;
	}
	return *this;
}

const std::string &Node::getClassName() const noexcept {
	static const std::string className("Node");
	return className;
}

std::ostream &operator<<(std::ostream &out, const Node &other) {
	const Object &otherObject = other;
	out << otherObject;
	out << "; children [";
	for (unsigned long i = 0; i < other.getChildren().size(); i++) {
		if (other.getChildren()[i] == nullptr) {
			out << "nullptr";
		} else {
			out << *other.getChildren()[i];
		}
		if (i < other.getChildren().size() - 1) {
			out << ", ";
		}
	}
	out << "]";
	return out;
}
