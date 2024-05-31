#pragma once

#include "device.h"

#include <string>
#include <vector>

namespace FH
{
	struct PipelineConfigInfo 
	{
		PipelineConfigInfo() = default;
		~PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo& other) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo& other) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		std::vector<VkDynamicState> dynamicStateEnables{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class FHPipeline
	{
	public:
		FHPipeline(FHDevice& device, const std::string& vertFilepath, 
			const std::string& fragFilepath, const PipelineConfigInfo& configInfo, bool is2D = false);
		~FHPipeline();
		FHPipeline(const FHPipeline&) = delete;
		FHPipeline& operator=(const FHPipeline&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

		static void DefaultPipelineConfigInfo(FH::PipelineConfigInfo& configInfo, bool is2D = false);

	private:
		static std::vector<char> ReadFile(const std::string& filePath);

		void CreateGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo, bool is2D);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		FHDevice& m_Device;
		VkShaderModule m_VertShaderModule{};
		VkShaderModule m_FragShaderModule{};
		VkPipeline m_GraphicsPipeline{};
	};
}