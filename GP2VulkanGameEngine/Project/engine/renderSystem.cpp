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

	struct PushConstantData2D
	{
		glm::mat2 transform{ 1.f };
		glm::vec2 offset;
		//Set alignment to prevent incorrect values in shader
		alignas(16)glm::vec3 color;
	};
}

/////////////////////////////
// 3D RENDERSYSTEM FUNCTIONS
/////////////////////////////

FH::FHRenderSystem::FHRenderSystem(
	FHDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
	: m_FHDevice{ device }
{
	CreatePipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);

	CreatePipelineLayout2D();
	CreatePipeline2D(renderPass);
}

FH::FHRenderSystem::~FHRenderSystem()
{
	vkDestroyPipelineLayout(m_FHDevice.GetDevice(), m_FHPipelineLayout, nullptr);
	vkDestroyPipelineLayout(m_FHDevice.GetDevice(), m_FHPipelineLayout2D, nullptr);

}

void FH::FHRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData3D);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(m_FHDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &m_FHPipelineLayout)
		!= VK_SUCCESS)
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

void FH::FHRenderSystem::RenderGameObjects(FHFrameInfo& frameInfo, std::vector<FHGameObject>& gameObjects)
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
		PushConstantData3D push{};
		push.modelMatrix = o.m_Transform.GetMatrix();
		push.normalMatrix = o.m_Transform.GetNormalMatrix();

		vkCmdPushConstants(
			frameInfo.m_CommandBuffer,
			m_FHPipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData3D),
			&push
		);
		o.m_Model->Bind(frameInfo.m_CommandBuffer);
		o.m_Model->Draw(frameInfo.m_CommandBuffer);
	}
}



/////////////////////////////
// 2D RENDERSYSTEM FUNCTIONS
/////////////////////////////

void FH::FHRenderSystem::CreatePipelineLayout2D()
{
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(PushConstantData2D);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(m_FHDevice.GetDevice(), &pipelineLayoutInfo, nullptr, &m_FHPipelineLayout2D) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout!");
}

void FH::FHRenderSystem::CreatePipeline2D(VkRenderPass renderPass)
{
	assert(m_FHPipelineLayout2D != nullptr && "Cannot create pipeline without pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	FHPipeline::DefaultPipelineConfigInfo(pipelineConfig, true);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = m_FHPipelineLayout2D;
	m_pFHPipeline2D = std::make_unique<FHPipeline>
		(m_FHDevice, "shaders/shader2D.vert.spv", "shaders/shader2D.frag.spv", pipelineConfig);
}

void FH::FHRenderSystem::RenderGameObjects2D(FHFrameInfo& frameInfo, std::vector<FHGameObject2D>& gameObjects)
{
	//update
	float i{};
	for (auto& o : gameObjects)
	{
		i += Time::GetDeltaTime();
		o.m_Transform2D.rotation = glm::mod(o.m_Transform2D.rotation + i * 0.1f, glm::two_pi<float>());
	}

	//render
	m_pFHPipeline2D->Bind(frameInfo.m_CommandBuffer);

	for (auto& o : gameObjects)
	{
		PushConstantData2D push{};
		push.offset = o.m_Transform2D.translation;
		push.color = o.m_Color;
		push.transform = o.m_Transform2D.GetMatrix();

		vkCmdPushConstants(
			frameInfo.m_CommandBuffer,
			m_FHPipelineLayout2D,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData2D),
			&push
		);
		o.m_Model->Bind(frameInfo.m_CommandBuffer);
		o.m_Model->Draw(frameInfo.m_CommandBuffer);
	}
}
