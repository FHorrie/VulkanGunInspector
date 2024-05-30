#pragma once

#include "device.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

namespace FH {

    class FHSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        FHSwapChain(FHDevice& deviceRef, VkExtent2D windowExtent);
        FHSwapChain(FHDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<FHSwapChain> previous);
        ~FHSwapChain();

        FHSwapChain(const FHSwapChain&) = delete;
        void operator=(const FHSwapChain&) = delete;

        uint32_t GetWidth() const { return m_SwapChainExtent.width; }
        uint32_t GetHeight() const { return m_SwapChainExtent.height; }
        VkFramebuffer GetFrameBuffer(int index) const { return m_SwapChainFramebuffers[index]; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        VkImageView GetImageView(int index) const { return m_SwapChainImageViews[index]; }
        size_t ImageCount() const { return m_SwapChainImages.size(); }
        VkFormat GetSwapChainImageFormat() const { return m_SwapChainImageFormat; }
        VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }

        float ExtentAspectRatio() 
        { return static_cast<float>(m_SwapChainExtent.width) 
                / static_cast<float>(m_SwapChainExtent.height);
        }
        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        bool CompareSwapFormats(const FHSwapChain& swapChain) const
        {
            return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat
                && swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
        }

        static VkImageView CreateImageView(FHDevice& device, VkImage image, VkFormat format);
    
    private:
        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        

        VkFormat m_SwapChainImageFormat;
        VkFormat m_SwapChainDepthFormat;
        VkExtent2D m_SwapChainExtent;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemories;
        std::vector<VkImageView> m_DepthImageViews;
        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        FHDevice& m_FHDevice;
        VkExtent2D m_WindowExtent2D;

        VkSwapchainKHR m_SwapChain;
        std::shared_ptr<FHSwapChain> m_OldSwapChain{};

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;
    };

}