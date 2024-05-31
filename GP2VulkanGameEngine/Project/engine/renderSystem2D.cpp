#include "renderSystem2D.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace FH
{
	struct SimplePushConstantData
	{
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		//Set alignment to prevent incorrect values in shader
		alignas(16)glm::vec3 color;
	};
}

FH::FHRenderSystem2D::FHRenderSystem2D(FHDevice& device, VkRenderPass renderPass)
	: m_FHDevice{ device }
{
	CreatePipelineLayout();
	CreatePipeline(renderPass);
}

FH::FHRenderSystem2D::~FHRenderSystem2D()
{
	vkDestroyPipelineLayout(m_FHDevice.GetDevice(), m_PipelineLayout, nullptr);
}

void FH::FHRenderSystem2D::CreatePipelineLayout()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(m_FHDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}

void FH::FHRenderSystem2D::CreatePipeline(VkRenderPass renderPass)
{
	assert(m_PipelineLayout != nullptr && "Cannot create pipeline without pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	FHPipeline::DefaultPipelineConfigInfo(pipelineConfig, true);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_PipelineLayout;
	m_pFHPipeline = std::make_unique<FHPipeline>
		(
			m_FHDevice,
			"shaders/shader2D.vert.spv",
			"shaders/shader2D.frag.spv",
			pipelineConfig,
			true
		);
}

void FH::FHRenderSystem2D::RenderGameObjects2D(VkCommandBuffer commandBuffer, std::vector<FHGameObject2D>& gameObjects)
{
	//update
	int i{};
	for (auto& o : gameObjects)
	{
		i += 1;
		o.m_Transform2D.rotation = glm::mod(o.m_Transform2D.rotation + 0.00001f * i, glm::two_pi<float>());
	}

	//render
	m_pFHPipeline->Bind(commandBuffer);

	for (auto& o : gameObjects)
	{
		SimplePushConstantData push{};
		push.offset = o.m_Transform2D.translation;
		push.color = o.m_Color;
		push.transform = o.m_Transform2D.GetMatrix();

		vkCmdPushConstants(
			commandBuffer,
			m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SimplePushConstantData),
			&push
		);
		o.m_Model->Bind(commandBuffer);
		o.m_Model->Draw(commandBuffer);
	}
}