#include "input/input.hpp"
#include "input/backend.hpp"

#include <common/json.hpp>

#include <fstream>
#include <iostream>

namespace eng
{

InputManager::InputManager(std::unique_ptr<IInputBackend>&& backend, const std::string& input_map):
	backend_(std::move(backend))
{
	ParseJson(input_map);
}

void InputManager::Poll()
{
	backend_->Poll();
}

int InputManager::ParseJson(const std::string& json_name)
{
	std::ifstream json_file(json_name, std::ios::binary);
	if (!json_file) {
		std::cerr << "Invalid file \"" << json_name << "\"\n";
		return -1;
	}
	std::string file_contents((
		std::istreambuf_iterator<char>(json_file)),
		std::istreambuf_iterator<char>()
	);

	nlohmann::json contents = nlohmann::json::parse(file_contents, nullptr, false);

	if (contents.is_discarded()) {
		std::cerr << "No valid json for input map\n";
		return -1;
	}

	const auto& input_list = contents.find("input_list");
	if (!input_list->is_array()) {
		std::cerr << "JSON parse error: input_list should be an array!\n";
		return -2;
	}
	const auto& surface_list = contents.find("surface_list");
	if (!surface_list->is_array()) {
		std::cerr << "JSON parse error: surface_list should be an array!\n";
		return -2;
	}

	for (const auto& element: input_list.value()) {
		if (!element.is_object()) {
			std::cerr << "input_list element not object, skipping\n";
			continue;
		}

		const auto& name_json = element.find("name");
		if (name_json == element.end() || !(name_json.value().is_string())) {
			std::cerr << "Invalid input_list element (no name attribute)\n";
			continue;
		}

		const auto& bindings_json = element.find("bindings");
		if (bindings_json == element.end() || !(bindings_json.value().is_object())) {
			std::cerr << "Invalid input_list element (no bindings object)\n";
			continue;
		}

		const auto& binding_name = backend_->GetBindingName();

		const auto& backend_binding = bindings_json.value().find(binding_name);
		if (backend_binding == bindings_json.value().end() || !(backend_binding.value().is_string())) {
			std::cerr << "Invalid binding for " << binding_name << " in binding for " << name_json.value() << "\n";
			continue;
		}

		input_dict_.insert(
			std::make_pair(
				name_json.value(),
				backend_binding.value()
			)
		);
	}

	for (const auto& element: surface_list.value()) {
		if (!element.is_object()) {
			std::cerr << "surface_list element not object, skipping\n";
			continue;
		}

		const auto& name_json = element.find("name");
		if (name_json == element.end() || !(name_json.value().is_string())) {
			std::cerr << "Invalid surface_list element (no name attribute)";
			continue;
		}

		const auto& bindings_json = element.find("bindings");
		if (bindings_json == element.end() || !(bindings_json.value().is_object())) {
			std::cerr << "Invalid surface_list element (no bindings object)\n";
			continue;
		}

		const auto &binding_name = backend_->GetBindingName();

		const auto& backend_binding = bindings_json.value().find(binding_name);
		if (backend_binding == bindings_json.value().end() || !(backend_binding.value().is_string())) {
			std::cerr << "Invalid binding for " << binding_name << " in binding for " << name_json.value() << "\n";
			continue;
		}

		surface_dict_.insert(
			std::make_pair(
				name_json.value(),
				backend_binding.value()
			)
		);
	}

	return 0;
}

bool InputManager::GetState(const std::string& id)
{
	const auto& it = input_dict_.find(id);

	if (it == input_dict_.end()) {
		return false;
	}

	return backend_->GetState(it->second);
}

vec2f InputManager::GetSurface(const std::string &id)
{
	const auto &it = surface_dict_.find(id);

	if (it == surface_dict_.end()) {
		return vec2f(0.0f, 0.0f);
	}

	return backend_->GetSurface(it->second);
}

void InputManager::SetMouseTracking(bool enable)
{
	backend_->SetMouseTracking(enable);
}

}
