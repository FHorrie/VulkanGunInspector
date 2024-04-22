#pragma once
#include "model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace FH
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f ,1.f};
		glm::vec3 rotation{};

		glm::mat4 GetMatrix();
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

	class FHGameObject
	{
	public:
		static FHGameObject CreateGameObject()
		{
			static unsigned int currentId{};
			return FHGameObject{ currentId++ };
		}
		~FHGameObject() = default;
		FHGameObject(const FHGameObject&) = delete;
		FHGameObject& operator=(const FHGameObject&) = delete;
		FHGameObject(FHGameObject&&) = default;
		FHGameObject& operator=(FHGameObject&&) = default;

		unsigned int GetId() { return m_Id; }

		std::shared_ptr<FHModel> m_Model{};
		glm::vec3 m_Color{};
		TransformComponent m_Transform{};
	private:
		FHGameObject(unsigned int objectId)
			: m_Id{objectId} 
		{}

		unsigned int m_Id{};
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