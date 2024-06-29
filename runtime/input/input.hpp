#ifndef _INPUT_HPP_
#define _INPUT_HPP_

#include <unordered_map>
#include <string>
#include <memory>

#include "input/backend.hpp"

namespace eng
{

class InputManager
{
public:
	InputManager(std::unique_ptr<IInputBackend>&&, const std::string& input_map);

	void Poll();

	bool GetState(const std::string &id);
	vec2f GetSurface(const std::string &id);
	void SetMouseTracking(bool enable);

private:
	std::unordered_map<std::string, std::string> input_dict_;
	std::unordered_map<std::string, std::string> surface_dict_;

	int ParseJson(const std::string& json);

	std::unique_ptr<IInputBackend> backend_;
};

}

#endif