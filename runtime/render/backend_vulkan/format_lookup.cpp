#include "backend_vulkan/format_lookup.hpp"

namespace eng
{

const std::unordered_map<EFormat, VkFormat> fmt_map = {
    {EFormat::kFormat_R24G8, VK_FORMAT_D24_UNORM_S8_UINT},
    {EFormat::kFormat_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM},
    {EFormat::kFormat_R10G10B10A2, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
    {EFormat::kFormat_R16G16_SNORM, VK_FORMAT_R16G16_SNORM}
};

}