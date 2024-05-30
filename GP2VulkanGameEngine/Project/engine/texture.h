#pragma once
#include "buffer.h"

#include <string>
#include <memory>

namespace FH
{
	class FHTexture
	{
	public:
		FHTexture(FHDevice& device, const std::string& path);
		~FHTexture();

		FHTexture(const FHTexture&)				= delete;
		FHTexture& operator=(const FHTexture&)	= delete;
		FHTexture(FHTexture&&)					= default;
		FHTexture& operator=(FHTexture&&)		= default;

		VkSampler GetTextureSampler() const { return m_TextureSampler; }
		VkImageView GetTextureImageView() const { return m_TextureImageView; }
		VkImageLayout GetTextureImageLayout() const { return m_TextureImageLayout; }

	private:
		void CreateTextureFromImage(const std::string& path);
		void CreateTextureSampler();

		void CreateImage(VkFormat format, VkImageTiling tiling,
			VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

		int m_TexWidth{};
		int m_TexHeight{};
		int m_MipmapCount{};

		VkSampler m_TextureSampler{};
		VkImageView m_TextureImageView{};
		VkImageLayout m_TextureImageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

		VkDeviceMemory m_TextureImageMemory{};
		VkImage m_TextureImage{};

		FHDevice& m_FHDevice;
	};
}