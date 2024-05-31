#pragma once
#include "engine/device.h"
#include "engine/pipeline.h"
#include "engine/gameObject.h"

#include <memory>
#include <vector>

namespace FH
{
	class FHRenderSystem2D
	{
	public:
		FHRenderSystem2D(FHDevice& device, VkRenderPass renderPass);
		~FHRenderSystem2D();

		FHRenderSystem2D(const FHRenderSystem2D&) = delete;
		FHRenderSystem2D(FHRenderSystem2D&&) = default;
		FHRenderSystem2D& operator=(const FHRenderSystem2D&) = delete;
		FHRenderSystem2D& operator=(FHRenderSystem2D&&) = default;

		void RenderGameObjects2D(VkCommandBuffer commandBuffer, 
			std::vector<FHGameObject2D>& gameObjects);

	private:
		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass);

		FHDevice& m_FHDevice;
		VkPipelineLayout m_PipelineLayout{};
		std::unique_ptr<FHPipeline> m_pFHPipeline{};
	};
}