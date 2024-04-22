#pragma once

#include "engine/device.h"
#include "camera.h"
#include "engine/pipeline.h"
#include "engine/gameObject.h"
#include "engine/frameInfo.h"

#include <memory>
#include <vector>

namespace FH
{
	class FHRenderSystem
	{
	public:
		FHRenderSystem(FHDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~FHRenderSystem();

		FHRenderSystem(const FHRenderSystem&) = delete;
		FHRenderSystem(FHRenderSystem&&) = default;
		FHRenderSystem& operator=(const FHRenderSystem&) = delete;
		FHRenderSystem& operator=(FHRenderSystem&&) = default;

		void RenderGameObjects(FHFrameInfo& frameInfo, std::vector<FHGameObject>& gameObjects);
		
		void RenderGameObjects2D(FHFrameInfo& frameInfo, std::vector<FHGameObject2D>& gameObjects);

	private:
		void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void CreatePipeline(VkRenderPass renderPass);
		
		void CreatePipelineLayout2D();
		void CreatePipeline2D(VkRenderPass renderPass);

		FHDevice& m_FHDevice;
		VkPipelineLayout m_FHPipelineLayout{};
		VkPipelineLayout m_FHPipelineLayout2D{};
		std::unique_ptr<FHPipeline> m_pFHPipeline{};
		std::unique_ptr<FHPipeline> m_pFHPipeline2D{};
	};
}