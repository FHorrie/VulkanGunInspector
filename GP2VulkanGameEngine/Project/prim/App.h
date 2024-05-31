#pragma once

#include "engine/window.h"
#include "engine/device.h"
#include "engine/renderer.h"
#include "engine/gameObject.h"
#include "engine/buffer.h"
#include "engine/descriptors.h"
#include "engine/texture.h"

#include <memory>
#include <vector>
#include <array>

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

		void CycleModelLeft();
		void CycleModelRight();

		void PrintControls();

	private:
		void LoadGameObjects();
		void LoadGameObjects2D();

		FHWindow m_FHWindow{ WIDTH, HEIGHT, "Vulkan Gun Inspector - Horrie Finian - 2DAE09" };
		FHDevice m_FHDevice{ m_FHWindow };
		FHRenderer m_FHRenderer{ m_FHWindow, m_FHDevice };

		bool m_ModelRotate{};

		//Define pool after device
		std::unique_ptr<FHDescriptorPool> m_pAppPool{};

		std::vector<std::unique_ptr<FHGameObject>> m_Models{};
		int m_CurrentModelIdx{};

		std::vector<FHGameObject2D> m_Models2D{};

		std::unique_ptr<FHGameObject> m_DirLight{};
	};

	struct DirectionalLight
	{
		glm::vec4 m_Direction{};
		glm::vec4 m_Color{};
	};

	struct GlobalUbo
	{
		glm::mat4 m_Projection{ 1.f };
		glm::mat4 m_View{ 1.f };
		DirectionalLight m_DirectionalLight{};
		glm::vec3 m_CameraPos{};
	};
}