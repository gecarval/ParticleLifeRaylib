#ifndef NODE_HPP
#define NODE_HPP

#include "Object.hpp"
#include <vector>

class Node : virtual public Object {
  public:
	typedef std::vector<Object *>::iterator		  iterator;
	typedef std::vector<Object *>::const_iterator const_iterator;

  protected:
	std::vector<Object *> _children;

  public:
	virtual ~Node();
	explicit Node(const std::string &instanceName = "") noexcept;
	explicit Node(const Node &other) noexcept;
	Node &operator=(const Node &other) noexcept;

	// getter
	std::vector<Object *> getChildren() const noexcept;

	// find method
	std::vector<Object *> findChild(const std::string &instanceName) noexcept;
	std::vector<Object *> findChild(const unsigned long instanceID) noexcept;
	std::vector<const Object *>
	findChild(const std::string &instanceName) const noexcept;
	std::vector<const Object *>
	findChild(const unsigned long instanceID) const noexcept;
	std::vector<Object *> findClass(const std::string &className) noexcept;
	std::vector<const Object *>
	findClass(const std::string &className) const noexcept;

	// insertion and deletion
	void	insertChild(Object &child, const Node::iterator &it) noexcept;
	void	pushFrontChild(Object &child) noexcept;
	void	pushBackChild(Object &child) noexcept;
	Object *frontChild(void) noexcept;
	Object *backChild(void) noexcept;
	void	popFrontChild(void) noexcept;
	void	popBackChild(void) noexcept;
	void	deleteChild(const Node::iterator &it) noexcept;
	void	deleteChild(const std::string &instanceName) noexcept;
	void	deleteChild(const unsigned long instanceID) noexcept;
	void	eraseChild(const std::string &instanceName) noexcept;
	void	eraseChild(const unsigned long instanceID) noexcept;

	virtual const std::string &getClassName() const noexcept;
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
