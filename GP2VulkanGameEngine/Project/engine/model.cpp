#include "model.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define TINYOBJECTLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include <cassert>
#include <iostream>
#include <unordered_map>

namespace std {
	template<>
	struct hash<FH::FHModel::Vertex>
	{
		size_t operator()(const FH::FHModel::Vertex& vertex) const
		{
			size_t seed{};
			FH::HashCombine(seed, vertex.pos, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

//////////////////////
// MODEL 3D FUNCTIONS
//////////////////////

//ModelData function
void FH::FHModel::ModelData::LoadModel(const std::string& filePath)
{
	tinyobj::attrib_t attribute;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string error;

	if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &error, filePath.c_str()))
		throw std::runtime_error(error);

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& s : shapes)
		for (const auto& index : s.mesh.indices)
		{
			Vertex vertex{};
			if (index.vertex_index >= 0)
			{
				vertex.pos = { 
					attribute.vertices[3 * index.vertex_index + 0], 
					attribute.vertices[3 * index.vertex_index + 1], 
					attribute.vertices[3 * index.vertex_index + 2] 
				};
			}
			if (index.normal_index >= 0)
			{
				vertex.normal = {
					attribute.normals[3 * index.normal_index + 0],
					attribute.normals[3 * index.normal_index + 1],
					attribute.normals[3 * index.normal_index + 2]
				};
			}
			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
					attribute.texcoords[2 * index.texcoord_index + 0],
					attribute.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);

		}
}

FH::FHModel::FHModel(FHDevice& device, const ModelData& construction)
	: m_FHDevice{device}
{
	CreateVertexBuffers(construction.vertices);
	CreateIndexBuffers(construction.indices);
}

void FH::FHModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	m_VertexCount = static_cast<uint32_t>(vertices.size());
	assert(m_VertexCount >= 3 && "Vertex count must be at least 3 (1 triangle)");
	uint32_t vertexSize = sizeof(vertices[0]);
	VkDeviceSize bufferSize = vertexSize * m_VertexCount;

	FHBuffer stagingBuffer
	{
		m_FHDevice,
		vertexSize,
		m_VertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};
	
	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)vertices.data());
	//UnMap takes place in the buffers destructor

	m_pVertexBuffer = std::make_unique<FHBuffer>
		(
			m_FHDevice,
			vertexSize,
			m_VertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	m_FHDevice.CopyBuffer(stagingBuffer.GetBuffer(), m_pVertexBuffer->GetBuffer(), bufferSize);
}

void FH::FHModel::CreateIndexBuffers(const std::vector<uint32_t>& indices)
{
	m_IndexCount = static_cast<uint32_t>(indices.size());
	m_HasIndexBuffer = m_IndexCount > 0;
	if (!m_HasIndexBuffer) return;

	uint32_t indexSize = sizeof(indices[0]);
	VkDeviceSize bufferSize{ indexSize * m_IndexCount };

	FHBuffer stagingBuffer
	{
		m_FHDevice,
		indexSize,
		m_IndexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)indices.data());

	m_pIndexBuffer = std::make_unique<FHBuffer>
		(
			m_FHDevice,
			indexSize,
			m_IndexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	m_FHDevice.CopyBuffer(stagingBuffer.GetBuffer(), m_pIndexBuffer->GetBuffer(), bufferSize);
}

void FH::FHModel::CreateTextureBuffer(const VkImage& image, uint32_t imageSize)
{
	m_HasTextureBuffer = imageSize > 0;
	if (!m_HasTextureBuffer) return;

	VkDeviceSize bufferSize{ imageSize };

	FHBuffer stagingBuffer
	{
		m_FHDevice,
		imageSize,
		1,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)image);

	m_pTextureBuffer = std::make_unique<FHBuffer>
		(
			m_FHDevice,
			imageSize,
			1,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	m_FHDevice.CopyBuffer(stagingBuffer.GetBuffer(), m_pTextureBuffer->GetBuffer(), bufferSize);
}

std::unique_ptr<FH::FHModel> FH::FHModel::CreateModelFromFile(FHDevice& device, const std::string& filePath)
{
	ModelData data{};
	data.LoadModel(filePath);
	std::cout << "Vertex count: " << data.vertices.size() << std::endl;
	return std::make_unique<FHModel>(device, data);
}

void FH::FHModel::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[]{ m_pVertexBuffer->GetBuffer() };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (m_HasIndexBuffer)
		vkCmdBindIndexBuffer(commandBuffer, m_pIndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

}

void FH::FHModel::Draw(VkCommandBuffer commandBuffer)
{
	if (m_HasIndexBuffer)
		vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
	else
		vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
	
}

std::vector<VkVertexInputBindingDescription> FH::FHModel::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> FH::FHModel::Vertex::GetAttributeDescriptions()
{ //Matches vertex struct
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	//Position
	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) });
	//Color
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
	//Normal
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
	//UV
	attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

	return attributeDescriptions;
}

//////////////////////
// MODEL 2D FUNCTIONS
//////////////////////

FH::FHModel2D::FHModel2D(FHDevice& device, const std::vector<FH::FHModel2D::Vertex>& vertices)
	: m_FHDevice{ device }
{
	CreateVertexBuffers(vertices);
}

void FH::FHModel2D::CreateVertexBuffers(const std::vector<Vertex>& vertices)
{
	m_VertexCount = static_cast<uint32_t>(vertices.size());
	assert(m_VertexCount >= 3 && "Vertex count must be at least 3 (1 triangle)");
	uint32_t vertexSize = sizeof(vertices[0]);
	VkDeviceSize bufferSize = vertexSize * m_VertexCount;

	FHBuffer stagingBuffer
	{
		m_FHDevice,
		vertexSize,
		m_VertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)vertices.data());

	m_pVertexBuffer = std::make_unique<FHBuffer>
		(
			m_FHDevice,
			vertexSize,
			m_VertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

	m_FHDevice.CopyBuffer(stagingBuffer.GetBuffer(), m_pVertexBuffer->GetBuffer(), bufferSize);
}

void FH::FHModel2D::Bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[]{ m_pVertexBuffer->GetBuffer() };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

}

void FH::FHModel2D::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> FH::FHModel2D::Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> FH::FHModel2D::Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	//Position
	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, pos) });

	//Color
	attributeDescriptions.push_back({ 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });

	return attributeDescriptions;
}