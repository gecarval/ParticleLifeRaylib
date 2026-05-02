#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "../CanvasItem.hpp"

class Control : public CanvasItem {
  private:
  public:
	Control(const std::string &instanceName = "");
	Control(const Control &other);
	Control &operator=(const Control &other);
	virtual ~Control();

	virtual const std::string &getClassName() const noexcept override;
};

#endif // CONTROL_HPP
