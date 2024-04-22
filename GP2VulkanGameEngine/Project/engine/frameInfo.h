#pragma once
#include "camera.h"

#include <vulkan/vulkan.h>

namespace FH
{
	struct FHFrameInfo
	{
		int m_FrameIdx;
		VkCommandBuffer m_CommandBuffer;
		FHCamera& m_FHCamera;
		VkDescriptorSet m_GlobalDescriptorSet;
	};
}