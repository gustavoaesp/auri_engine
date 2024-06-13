#ifndef _INPUT_BACKEND_HPP_
#define _INPUT_BACKEND_HPP_

#include <string>
#include <math/vector.hpp>

namespace eng
{

class IInputBackend
{
public:
	virtual ~IInputBackend() {}

	virtual void Poll() = 0;

	virtual bool GetState(const std::string &id) = 0;
	virtual vec2f GetSurface(const std::string &id) = 0;

	virtual void SetMouseTracking(bool enabled) = 0;

	const std::string& GetBindingName() { return binding_name_; }

protected:
	IInputBackend(const std::string& binding_name):
		binding_name_(binding_name)
	{}
	std::string binding_name_;
};

}

#endif