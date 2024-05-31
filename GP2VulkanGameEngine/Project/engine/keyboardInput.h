#pragma once

#include "gameObject.h"
#include "window.h"

namespace FH
{
	class FirstApp;

	class KeyboardInput
	{
	public:
		struct KeyMappings
		{
			int moveForward{ GLFW_KEY_W };
			int moveBackward{ GLFW_KEY_S };
			int moveLeft{ GLFW_KEY_A };
			int moveRight{ GLFW_KEY_D };
			int moveUp{ GLFW_KEY_E };
			int moveDown{ GLFW_KEY_Q };

			int toggleModelRotate{GLFW_KEY_F5};
			int cycleModelLeft{GLFW_KEY_LEFT};
			int cycleModelRight{GLFW_KEY_RIGHT};
		};

		void MoveInPlaneXZ(GLFWwindow* window, FHGameObject& gameObject);
		void UpdateSceneInputs(GLFWwindow* window, FirstApp* app);

	private:
		KeyMappings m_Keys{};
		float m_MoveSpeed{ 3.f };
		float m_TurnSpeed{ 6.f };

		inline static bool FIRST_PASS{ true };
		double m_PreviousXPos{}, m_PreviousYPos{};

		bool m_RotateButtonPressed{};
		bool m_LeftButtonPressed{};
		bool m_RightButtonPressed{};
	};
}
