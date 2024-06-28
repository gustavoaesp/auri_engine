#include "managers/shader_list.hpp"

#include <filesystem>
#include <iostream>

#include "primitives/shader.hpp"
#include "renderbackend.hpp"

namespace eng
{

RShaderList::RShaderList(IRenderBackend *backend)
{
    std::filesystem::path folder("shaders/");
    std::filesystem::directory_iterator iterator(folder);
    for (auto &file :iterator) {
        std::string filename = file.path().string();
        RShaderPipelineBind pipeline_bind;
        if (filename.find("vert") != std::string::npos) {
            pipeline_bind = RShaderPipelineBind::kShaderVertex;
        } else if(filename.find("frag") != std::string::npos) {
            pipeline_bind = RShaderPipelineBind::kShaderFragment;
        }

        shaders_[filename] = std::unique_ptr<RShader>(
            backend->CreateShader(filename.c_str(), pipeline_bind)
        );
    }
}

RShader *RShaderList::Get(const std::string &key)
{
    const auto &find_element = shaders_.find(key);
    if (find_element == shaders_.end()) {
        return nullptr;
    }
    return find_element->second.get();
}

}