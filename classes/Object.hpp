#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <ostream>

class Object {
  private:
	static unsigned long _instanceIDCounter;
	const unsigned long	 _instanceID;

  protected:
	std::string _instanceName;

  public:
	virtual ~Object();
	explicit Object(const std::string &instanceName = "");
	explicit Object(const Object &other);
	Object &operator=(const Object &other) noexcept;

	unsigned long	   getInstanceID() const noexcept;
	const std::string &getInstanceName() const noexcept;
	std::string		  &getInstanceName() noexcept;
	void setInstanceName(const std::string &instanceName = "") noexcept;

	// operators
	bool operator==(const Object &other) const noexcept;
	bool operator!=(const Object &other) const noexcept;

	virtual const std::string &getClassName() const noexcept;
};

std::ostream &operator<<(std::ostream &out, const Object &other);

#endif
