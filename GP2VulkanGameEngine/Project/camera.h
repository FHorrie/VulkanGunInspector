#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace FH
{
	class FHCamera
	{
	public:
		void SetOrthographicProjection(
			float left, float right, float top, float bottom, float near, float far);

		void SetPerspectiveProjection(float FOV, float aspect, float near, float far);

		void SetViewDirection(glm::vec3 pos, glm::vec3 direction, glm::vec3 up = { 0.f, -1.f, 0.f });
		
		void SetViewTarget(glm::vec3 pos, glm::vec3 target, glm::vec3 up = { 0.f, -1.f, 0.f })
		{ SetViewDirection(pos, target - pos, up); }

		void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		
	private:
		void CalculateViewMatrix(
			const glm::vec3& u, const glm::vec3& v, const glm::vec3& w, const glm::vec3& pos);

		glm::mat4 m_ProjectionMatrix{ 1.f };
		glm::mat4 m_ViewMatrix{};
	};
}