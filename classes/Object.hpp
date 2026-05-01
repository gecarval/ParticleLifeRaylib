#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>

class Object {
  private:
	static unsigned long _instanceIDCounter;

	const unsigned long _instanceID;
	std::string			_instanceName;

  public:
	virtual ~Object();
	explicit Object(const std::string &instanceName = "");
	explicit Object(const Object &other);
	Object &operator=(const Object &other);

	unsigned long	   getInstanceID(void) const noexcept;
	const std::string &getInstanceName(void) const noexcept;
	std::string		  &getInstanceName(void) noexcept;
	void setInstanceName(const std::string &instanceName = "") noexcept;

	// operators
	bool operator==(const Object &other) const noexcept;
	bool operator!=(const Object &other) const noexcept;

	virtual const std::string &getClassName(void) const noexcept;
};

#endif
