#pragma once

#include "engine/window.h"
#include "engine/device.h"
#include "engine/renderer.h"
#include "engine/gameObject.h"
#include "engine/buffer.h"
#include "engine/descriptors.h"

#include <memory>
#include <vector>

namespace FH
{
	class FirstApp
	{
	public:
		static inline constexpr int WIDTH{ 800 };
		static inline constexpr int HEIGHT{ 600 };

		FirstApp();
		~FirstApp() = default;
		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void Run();

		void ToggleModelRotate() { m_ModelRotate = !m_ModelRotate; }

		void PrintControls();

	private:
		void LoadGameObjects();
		void LoadGameObjects2D();

		FHWindow m_FHWindow{ WIDTH, HEIGHT, "GP2VulkanEngine - Horrie Finian - 2DAE09" };
		FHDevice m_FHDevice{ m_FHWindow };
		FHRenderer m_FHRenderer{ m_FHWindow, m_FHDevice };

		bool m_ModelRotate{};

		//Define pool after device
		std::unique_ptr<FHDescriptorPool> m_pAppPool{};
		std::vector<FHGameObject> m_GameObjects{};
		std::vector<FHGameObject2D> m_GameObjects2D{};
	};

	struct UniBufferObj
	{
		glm::mat4 m_ProjectionView{1.f};
		glm::vec3 m_LightDirection{ glm::normalize(glm::vec3{1.f, -3.f, -1.f}) };
	};
}