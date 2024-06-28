#ifndef _RENDER_MANAGERS_SHADER_LIST_HPP_
#define _RENDER_MANAGERS_SHADER_LIST_HPP_

#include <memory>
#include <string>
#include <unordered_map>

namespace eng
{

class IRenderBackend;
class RShader;

class RShaderList
{
public:
    RShaderList(IRenderBackend *);
    RShader *Get(const std::string&);
protected:
    std::unordered_map<std::string, std::unique_ptr<RShader>> shaders_;
};

}

#endif