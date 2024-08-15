#ifndef _RENDER_TEXTURE_MANAGER_HPP_
#define _RENDER_TEXTURE_MANAGER_HPP_

#include "managers/manager_template.hpp"
#include "primitives/texture.hpp"
#include "renderbackend.hpp"

namespace eng
{

class RTextureManager : public TResourceManager<RTexture>
{
public:
    RTextureManager(IRenderBackend *);
protected:
    RTexture *Load(const char*) override;
};

}

#endif