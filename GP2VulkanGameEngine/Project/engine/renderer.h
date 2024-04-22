#pragma once

#include "engine/window.h"
#include "engine/device.h"
#include "engine/swapchain.h"

#include <memory>
#include <vector>
#include <cassert>

namespace FH
{
	class FHRenderer
	{
	public:
		FHRenderer(FHWindow& window, FHDevice& device);
		~FHRenderer();

		FHRenderer(const FHRenderer&) = delete;
		FHRenderer& operator=(const FHRenderer&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return m_pFHSwapChain->GetRenderPass(); }
		float GetAspectRatio() const { return m_pFHSwapChain->ExtentAspectRatio(); }
		bool IsFrameInProgress() const { return m_IsFrameStarted; }
		VkCommandBuffer GetCurrentCommandBuffer() const 
		{
			assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_CommandBuffers[m_CurrentFrameIdx];
		}

		int GetFrameIndex() const
		{
			assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
			return m_CurrentFrameIdx;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();

		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void RecreateSwapChain();
		void CreateCommandBuffers();
		void FreeCommandBuffers();

		FHWindow& m_FHWindow;
		FHDevice& m_FHDevice;
		std::unique_ptr<FHSwapChain> m_pFHSwapChain{};
		std::vector<VkCommandBuffer> m_CommandBuffers{};

		uint32_t m_CurrentImageIdx{};
		int m_CurrentFrameIdx{};
		bool m_IsFrameStarted{};
	};
}