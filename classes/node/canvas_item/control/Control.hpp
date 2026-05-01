#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "../CanvasItem.hpp"

class Control : virtual public CanvasItem {
  private:
  public:
	Control(const std::string &instanceName = "");
	Control(const Control &other);
	Control &operator=(const Control &other);
	virtual ~Control();

	virtual const std::string &getClassName() const noexcept;
};

#endif // CONTROL_HPP
