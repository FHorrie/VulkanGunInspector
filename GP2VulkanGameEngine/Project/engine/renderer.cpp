#include "renderer.h"

#include <stdexcept>
#include <array>

FH::FHRenderer::FHRenderer(FHWindow& window, FHDevice& device)
	: m_FHWindow{window}
	, m_FHDevice{device}
{
	RecreateSwapChain();
	CreateCommandBuffers();
}

FH::FHRenderer::~FHRenderer()
{
	FreeCommandBuffers();
}

void FH::FHRenderer::RecreateSwapChain()
{
	auto extent{ m_FHWindow.GetExtent() };
	while (extent.width == 0 || extent.height == 0)
	{
		extent = m_FHWindow.GetExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_FHDevice.GetDevice());
	if (m_pFHSwapChain == nullptr)
		m_pFHSwapChain = std::make_unique<FHSwapChain>(m_FHDevice, extent);
	else
	{
		FHSwapChain* oldSwapChain = m_pFHSwapChain.get();
		m_pFHSwapChain = std::make_unique<FHSwapChain>(m_FHDevice, extent, std::move(m_pFHSwapChain));

		if (!oldSwapChain->CompareSwapFormats(*m_pFHSwapChain.get()))
			throw std::runtime_error("Swapchain image/depth format has changed!");
	}
}

void FH::FHRenderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(FHSwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_FHDevice.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_FHDevice.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers!");

}

void FH::FHRenderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(
		m_FHDevice.GetDevice(),
		m_FHDevice.GetCommandPool(),
		static_cast<float>(m_CommandBuffers.size()),
		m_CommandBuffers.data()
	);
	m_CommandBuffers.clear();
}

VkCommandBuffer FH::FHRenderer::BeginFrame()
{
	assert(!m_IsFrameStarted && "Can't begin frame while previous frame in progress");

	auto result = m_pFHSwapChain->AcquireNextImage(&m_CurrentImageIdx);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return nullptr;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("failed to acquire swap chain image");
	
	m_IsFrameStarted = true;
	auto commandBuffer = GetCurrentCommandBuffer();

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffers!");

	return commandBuffer;
}

void FH::FHRenderer::EndFrame()
{
	assert(m_IsFrameStarted && "Can't end frame while frame not in progress");

	auto commandBuffer = GetCurrentCommandBuffer();
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer");

	auto result = m_pFHSwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIdx);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FHWindow.IsWindowResized())
	{
		m_FHWindow.ResetWindowResizedFlag();
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("failed to present swap chain image");

	m_IsFrameStarted = false;
	m_CurrentFrameIdx = (m_CurrentFrameIdx + 1) % FHSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void FH::FHRenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(m_IsFrameStarted && "Can't begin render pass while frame not in progress");
	assert(commandBuffer == GetCurrentCommandBuffer() &&
		"Can't begin render pass while passed command buffer is not the current one");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_pFHSwapChain->GetRenderPass();
	renderPassInfo.framebuffer = m_pFHSwapChain->GetFrameBuffer(m_CurrentImageIdx);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_pFHSwapChain->GetSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.f };
	//clearValues[0].depthStencil = ?; //Ignored vals
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_pFHSwapChain->GetSwapChainExtent().width);
	viewport.height = static_cast<float>(m_pFHSwapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{
		{0, 0},
		m_pFHSwapChain->GetSwapChainExtent()
	};

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void FH::FHRenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(m_IsFrameStarted && "Can't end render pass while frame not in progress");
	assert(commandBuffer == GetCurrentCommandBuffer() &&
		"Can't end render pass while passed command buffer is not the current one");

	vkCmdEndRenderPass(commandBuffer);
}