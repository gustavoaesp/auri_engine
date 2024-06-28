#include "input/backends/glfw.hpp"
#include "input/backend.hpp"

#include <iostream>

#include <GLFW/glfw3.h>
#include <unordered_map>

static const std::unordered_map<std::string, uint32_t> glfw_mappings_ = {
	{"W", GLFW_KEY_W},
	{"A", GLFW_KEY_A},
	{"S", GLFW_KEY_S},
	{"D", GLFW_KEY_D},
	{"E", GLFW_KEY_E},
	{"Q", GLFW_KEY_Q},
	{"space", GLFW_KEY_SPACE},
	{"esc", GLFW_KEY_ESCAPE}
};

namespace eng
{

static float callback_cursor_x = 0;
static float callback_cursor_y = 0;

static vec2f curr_diff;

static bool first_frame = true;

static void mouse_callback(GLFWwindow *window, double nx, double ny)
{
	if (first_frame) {
		first_frame = 0;
		return;
	}
	curr_diff(0) = (nx - callback_cursor_x);
	curr_diff(1) = (ny - callback_cursor_y);

	callback_cursor_x = nx;
	callback_cursor_y = ny;
}

GLFWInput::GLFWInput(GLFWwindow* window):
	window_ref_(window), IInputBackend("PC"),
	mouse_tracking_enabled_(false)
{}

GLFWInput::~GLFWInput()
{}

void GLFWInput::Poll()
{
	curr_diff = vec2f();
	glfwPollEvents();
}

bool GLFWInput::GetState(const std::string& id)
{
	const auto& it = glfw_mappings_.find(id);
	if (it == glfw_mappings_.end()) {
		return false;
	}

	return (glfwGetKey(window_ref_, it->second) == GLFW_PRESS);
}

vec2f GLFWInput::GetSurface(const std::string &id)
{
	if (!mouse_tracking_enabled_) {
		 return vec2f(0.0f, 0.0f);
	}

	if (id == "mouse_xy") {
		return curr_diff;
	}

	return vec2f(0.0f, 0.0f);
}

void GLFWInput::SetMouseTracking(bool enable)
{
	if (enable == mouse_tracking_enabled_) return;

	mouse_tracking_enabled_ = enable;

	if (enable) {
		//glfwSetCursorPos(window_ref_, 800, 450);
		glfwSetInputMode(window_ref_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported()) {
			glfwSetInputMode(window_ref_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		glfwSetCursorPosCallback(window_ref_, &mouse_callback);
	} else {
		glfwSetInputMode(window_ref_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetCursorPosCallback(window_ref_, nullptr);
	}
}

}
