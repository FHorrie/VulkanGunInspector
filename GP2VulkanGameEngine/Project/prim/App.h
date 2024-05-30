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

		void PrintControls();

	private:
		void LoadGameObjects();
		//void LoadGameObjects2D();

		FHWindow m_FHWindow{ WIDTH, HEIGHT, "BULKAN - Horrie Finian - 2DAE09" };
		FHDevice m_FHDevice{ m_FHWindow };
		FHRenderer m_FHRenderer{ m_FHWindow, m_FHDevice };

		bool m_ModelRotate{};

		//Define pool after device
		std::unique_ptr<FHDescriptorPool> m_pAppPool{};
		std::vector<std::unique_ptr<FHGameObject>> m_Models{};
		std::vector<std::unique_ptr<FHGameObject>> m_DirLights{};

		//std::vector<FHGameObject2D> m_GameObjects2D{};;
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
		glm::vec4 m_AmbientLightColor{ 1.f, 1.f, 1.f, 0.02f/*-> light intensity*/};
		DirectionalLight m_DirectionalLights[3]{};
		int m_DirectionalLightAmount{};
	};
}