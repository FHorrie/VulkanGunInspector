#pragma once
#include "model.h"
#include "texture.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace FH
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f ,1.f};
		glm::vec3 rotation{};

		glm::mat4 GetModelMatrix();
		glm::mat3 GetNormalMatrix();
	};

	struct TransformComponent2D
	{
		glm::vec2 translation{};
		glm::vec2 scale{ 1.f, 1.f };
		float rotation{};

		glm::mat2 GetMatrix()
		{
			const float sinVal = glm::sin(rotation);
			const float cosVal = glm::cos(rotation);
			//ROW MAJOR MATRICES
			glm::mat2 rotationMat
			{
				{cosVal, sinVal},
				{-sinVal, cosVal}
			};

			glm::mat2 scaleMat
			{
				{scale.x, 0.f},
				{0.f, scale.y}
			};
			return glm::mat2{ rotationMat * scaleMat };
		}
	};

	struct DirectionalLightComponent
	{
		float lightIntensity{};
		glm::vec3 direction{};
	};

	class FHGameObject
	{
	public:
		static FHGameObject CreateGameObject()
		{
			static unsigned int currentId{};
			return FHGameObject{ currentId++ };
		}

		static FHGameObject CreateDirectionalLight(
			float intensity = 1.f,
			glm::vec3 direction = glm::vec3{ 1.f, 3.f, 1.f }, 
			glm::vec3 color = glm::vec3(1.f)
		);

		~FHGameObject() = default;
		FHGameObject(const FHGameObject&) = delete;
		FHGameObject& operator=(const FHGameObject&) = delete;
		FHGameObject(FHGameObject&&) = default;
		FHGameObject& operator=(FHGameObject&&) = default;

		unsigned int GetId() { return m_Id; }

		std::unique_ptr<FHModel> m_Model{};

		std::unique_ptr<FHTexture> m_DiffuseTexture{};
		std::unique_ptr<FHTexture> m_NormalTexture{};
		std::unique_ptr<FHTexture> m_RoughnessTexture{};
		std::unique_ptr<FHTexture> m_SpecularTexture{};
		std::unique_ptr<FHTexture> m_AOTexture{};

		std::unique_ptr<DirectionalLightComponent> m_DirLightComp{ nullptr };
		glm::vec3 m_Color{};

		TransformComponent m_Transform{};

		VkDescriptorSet GetDescriptorSetAtFrame(int frame) const 
		{
			return m_ObjectDescriptorSets[frame];
		}

		void SetDescriptorSetAtFrame(int frame, VkDescriptorSet descriptorSet);

	private:
		FHGameObject(uint32_t objectId);

		uint32_t m_Id{};
		std::vector<VkDescriptorSet> m_ObjectDescriptorSets;
	};

	class FHGameObject2D
	{
	public:
		static FHGameObject2D CreateGameObject()
		{
			static unsigned int currentId{};
			return FHGameObject2D{ currentId++ };
		}
		~FHGameObject2D() = default;
		FHGameObject2D(const FHGameObject2D&) = delete;
		FHGameObject2D& operator=(const FHGameObject2D&) = delete;
		FHGameObject2D(FHGameObject2D&&) = default;
		FHGameObject2D& operator=(FHGameObject2D&&) = default;

		unsigned int GetId() { return m_Id; }

		std::shared_ptr<FHModel2D> m_Model{};
		glm::vec3 m_Color{};
		TransformComponent2D m_Transform2D{};
	private:
		FHGameObject2D(unsigned int objectId)
			: m_Id{ objectId }
		{}

		unsigned int m_Id{};
	};
}