#pragma once

#include "input/backend.hpp"

#include <GLFW/glfw3.h>

namespace eng
{

class GLFWInput : public IInputBackend
{
public:
	GLFWInput(GLFWwindow*);
	~GLFWInput() override;

	void Poll() override;
	bool GetState(const std::string &id) override;
	vec2f GetSurface(const std::string &id) override;

	void SetMouseTracking(bool enabled) override;
private:
	GLFWwindow* window_ref_;

	bool mouse_tracking_enabled_;
};

}
