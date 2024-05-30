#pragma once
#include "buffer.h"
#include "device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>

namespace FH
{
	class FHModel
	{
	public:

		struct Vertex
		{
			glm::vec3 pos{};
			glm::vec3 color{1.f};
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

			bool operator==(const Vertex& other) const 
			{
				return pos == other.pos && 
					color == other.color &&
					normal == other.normal &&
					uv == other.uv;
			}
		};

		struct ModelData
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void LoadModel(const std::string& filePath);
		};

		FHModel(FHDevice& device, const ModelData& construction);
		~FHModel() = default;

		FHModel(const FHModel&) = delete;
		FHModel& operator=(const FHModel&) = delete;

		static std::unique_ptr<FHModel> CreateModelFromFile(
			FHDevice& device, const std::string& filePath);

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);

		FHDevice& m_FHDevice;

		std::unique_ptr<FHBuffer> m_pVertexBuffer;
		uint32_t m_VertexCount = 0;

		bool m_HasIndexBuffer{ false };
		std::unique_ptr<FHBuffer> m_pIndexBuffer;
		uint32_t m_IndexCount = 0;
	};

	class FHModel2D
	{
	public:

		struct Vertex2D
		{
			glm::vec2 pos;
			glm::vec3 color;

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
		};

		FHModel2D(FHDevice& device, const std::vector<Vertex2D>& vertices);
		~FHModel2D() = default;

		FHModel2D(const FHModel2D&) = delete;
		FHModel2D& operator=(const FHModel2D&) = delete;

		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex2D>& vertices);

		FHDevice& m_FHDevice;

		std::unique_ptr<FHBuffer> m_pVertexBuffer;
		uint32_t m_VertexCount = 0;
	};
}