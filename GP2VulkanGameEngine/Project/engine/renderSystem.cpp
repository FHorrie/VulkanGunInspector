#include "renderSystem.h"
#include "FHTime.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace FH
{
	struct PushConstantData3D
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};
}

FH::FHRenderSystem::FHRenderSystem(
	FHDevice& device, VkRenderPass renderPass, 
	const std::vector<VkDescriptorSetLayout>& globalSetLayouts)
	: m_FHDevice{ device }
{
	CreatePipelineLayout(globalSetLayouts);
	CreatePipeline(renderPass);
}

FH::FHRenderSystem::~FHRenderSystem()
{
	vkDestroyPipelineLayout(m_FHDevice.GetDevice(), m_FHPipelineLayout, nullptr);

}

void FH::FHRenderSystem::CreatePipelineLayout(
	const std::vector<VkDescriptorSetLayout>& globalSetLayouts)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData3D);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayouts };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_FHDevice.GetDevice(), &pipelineLayoutInfo, 
		nullptr, &m_FHPipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}

void FH::FHRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	assert(m_FHPipelineLayout != nullptr && "Cannot create pipeline without pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	FHPipeline::DefaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_FHPipelineLayout;
	m_pFHPipeline = std::make_unique<FHPipeline>
		(m_FHDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void FH::FHRenderSystem::RenderGameObjects(FHFrameInfo& frameInfo, 
	std::vector<FHGameObject*>& gameObjects)
{
	m_pFHPipeline->Bind(frameInfo.m_CommandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.m_CommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_FHPipelineLayout,
		0, 1,
		&frameInfo.m_GlobalDescriptorSet,
		0,
		nullptr
	);

	for (auto& o : gameObjects)
	{
		// Bind descriptor set for access to object specific textures
		VkDescriptorSet objectDescriptorSet = o->GetDescriptorSetAtFrame(frameInfo.m_FrameIdx);
		vkCmdBindDescriptorSets(
			frameInfo.m_CommandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, 
			m_FHPipelineLayout,
			1, 1, 
			&objectDescriptorSet, 
			0, 
			nullptr
		);

		PushConstantData3D push{};
		push.modelMatrix = o->m_Transform.GetModelMatrix();
		push.normalMatrix = o->m_Transform.GetNormalMatrix();

		vkCmdPushConstants(
			frameInfo.m_CommandBuffer,
			m_FHPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData3D),
			&push
		);

		o->m_Model->Bind(frameInfo.m_CommandBuffer);
		o->m_Model->Draw(frameInfo.m_CommandBuffer);
	}
}

void FH::FHRenderSystem::RenderGameObject(FHFrameInfo& frameInfo,
	FHGameObject* gameObject)
{
	m_pFHPipeline->Bind(frameInfo.m_CommandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.m_CommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_FHPipelineLayout,
		0, 1,
		&frameInfo.m_GlobalDescriptorSet,
		0,
		nullptr
	);

	// Bind descriptor set for access to object specific textures
	VkDescriptorSet objectDescriptorSet = gameObject->GetDescriptorSetAtFrame(frameInfo.m_FrameIdx);
	vkCmdBindDescriptorSets(
		frameInfo.m_CommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_FHPipelineLayout,
		1, 1,
		&objectDescriptorSet,
		0,
		nullptr
	);

	PushConstantData3D push{};
	push.modelMatrix = gameObject->m_Transform.GetModelMatrix();
	push.normalMatrix = gameObject->m_Transform.GetNormalMatrix();

	vkCmdPushConstants(
		frameInfo.m_CommandBuffer,
		m_FHPipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(PushConstantData3D),
		&push
	);

	gameObject->m_Model->Bind(frameInfo.m_CommandBuffer);
	gameObject->m_Model->Draw(frameInfo.m_CommandBuffer);
}
