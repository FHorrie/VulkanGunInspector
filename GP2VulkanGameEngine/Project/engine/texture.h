#pragma once
#include "buffer.h"

#include <string>
#include <memory>

namespace FH
{
	class FHTexture
	{
	public:
		FHTexture(const std::string& path);
		~FHTexture() = default;

		FHTexture(const FHTexture&)				= delete;
		FHTexture& operator=(const FHTexture&)	= delete;
		FHTexture(FHTexture&&)					= default;
		FHTexture& operator=(FHTexture&&)		= default;

	private:
		void CreateTextureFromImage(const std::string& path);

		VkImage m_TextureImage{};
		VkDeviceMemory m_TextureImageMemory{};

		FHDevice& m_FHDevice;
	};
}