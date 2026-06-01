#ifndef NODE_HPP
#define NODE_HPP

#include "../Object.hpp"
#include <vector>

class Node : public Object {
  public:
	typedef std::vector<Node *>::iterator		iterator;
	typedef std::vector<Node *>::const_iterator const_iterator;

  protected:
	Node			   *_parent;
	std::vector<Node *> _children;

  public:
	virtual ~Node();
	explicit Node(const std::string &instanceName = "") noexcept;
	explicit Node(const Node &other) noexcept;
	Node &operator=(const Node &other) noexcept;

	// getter and setter
	Node			   &setParent(Node *parent) noexcept;
	Node			   *getParent() const noexcept;
	std::vector<Node *> getChildren() const noexcept;

	// State
	virtual void ready() noexcept;
	virtual void update() noexcept;
	virtual void updatePhysics() noexcept;

	// find method
	std::vector<Node *> findChild(const std::string &instanceName) noexcept;
	std::vector<Node *> findChild(const unsigned long instanceID) noexcept;
	std::vector<const Node *>
	findChild(const std::string &instanceName) const noexcept;
	std::vector<const Node *>
	findChild(const unsigned long instanceID) const noexcept;
	std::vector<Node *> findClass(const std::string &className) noexcept;
	std::vector<const Node *>
	findClass(const std::string &className) const noexcept;

	// insertion and deletion
	Node &insertChild(Node &child, const Node::iterator &it) noexcept;
	Node &pushFrontChild(Node &child) noexcept;
	Node &pushBackChild(Node &child) noexcept;
	Node *frontChild() noexcept;
	Node *backChild() noexcept;
	Node &popFrontChild() noexcept;
	Node &popBackChild() noexcept;
	Node &deleteChild(const Node::iterator &it) noexcept;
	Node &deleteChild(const std::string &instanceName) noexcept;
	Node &deleteChild(const unsigned long instanceID) noexcept;
	Node &removeChild(const Node::iterator &it) noexcept;
	Node &removeChild(const std::string &instanceName) noexcept;
	Node &removeChild(const unsigned long instanceID) noexcept;

	virtual const std::string &getClassName() const noexcept override;
};

std::ostream &operator<<(std::ostream &out, const Node &other);

// Template function for print vectors
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &vec) {
	out << "[";
	for (size_t i = 0; i < vec.size(); i++) {
		out << vec[i];
		if (i < vec.size() - 1) {
			out << ", ";
		}
	}
	out << "]";
	return (out);
}
#endif
