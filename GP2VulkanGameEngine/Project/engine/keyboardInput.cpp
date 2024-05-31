#include "keyboardInput.h"
#include "FHTime.h"
#include "prim/App.h"

void FH::KeyboardInput::MoveInPlaneXZ(GLFWwindow* window, FHGameObject& gameObject)
{
	double xPos{}, yPos{};
	glfwGetCursorPos(window, &xPos, &yPos);

	glm::vec3 rotation{0.f};
	
	int lmbState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (lmbState == GLFW_PRESS)
		m_MoveSpeed = 9.f;
	else
		m_MoveSpeed = 3.f;
	
	int rmbState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (rmbState == GLFW_PRESS)
	{
		rotation.y += static_cast<float>(xPos - m_PreviousXPos);
		rotation.x -= static_cast<float>(yPos - m_PreviousYPos);
	}

	m_PreviousXPos = xPos;
	m_PreviousYPos = yPos;

	if(glm::dot(rotation, rotation) > 0.0001f)
		gameObject.m_Transform.rotation += m_TurnSpeed * Time::GetDeltaTime() * rotation;

	gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, glm::radians(-89.f), glm::radians(89.f));
	gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());

	const float yaw{ gameObject.m_Transform.rotation.y };
	const glm::vec3 forward{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 right{ forward.z, 0.f, -forward.x };
	const glm::vec3 up{ 0.f, -1.f, 0.f };

	glm::vec3 moveDirection{0.f};
	if (glfwGetKey(window, m_Keys.moveForward) == GLFW_PRESS) moveDirection += forward;
	if (glfwGetKey(window, m_Keys.moveBackward) == GLFW_PRESS) moveDirection -= forward;
	if (glfwGetKey(window, m_Keys.moveRight) == GLFW_PRESS) moveDirection += right;
	if (glfwGetKey(window, m_Keys.moveLeft) == GLFW_PRESS) moveDirection -= right;
	if (glfwGetKey(window, m_Keys.moveUp) == GLFW_PRESS) moveDirection += up;
	if (glfwGetKey(window, m_Keys.moveDown) == GLFW_PRESS) moveDirection -= up;

	if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
		gameObject.m_Transform.translation += m_MoveSpeed * Time::GetDeltaTime() * glm::normalize(moveDirection);
}

void FH::KeyboardInput::UpdateSceneInputs(GLFWwindow* window, FirstApp* app)
{
	if (glfwGetKey(window, m_Keys.toggleModelRotate) == GLFW_PRESS && !m_RotateButtonPressed)
	{
		app->ToggleModelRotate();
		m_RotateButtonPressed = true;
	}
	else if (glfwGetKey(window, m_Keys.toggleModelRotate) == GLFW_RELEASE && m_RotateButtonPressed)
		m_RotateButtonPressed = false;

	if (glfwGetKey(window, m_Keys.cycleModelLeft) == GLFW_PRESS && !m_LeftButtonPressed)
	{
		app->CycleModelLeft();
		m_LeftButtonPressed = true;
	}
	else if (glfwGetKey(window, m_Keys.cycleModelLeft) == GLFW_RELEASE && m_LeftButtonPressed)
		m_LeftButtonPressed = false;

	if (glfwGetKey(window, m_Keys.cycleModelRight) == GLFW_PRESS && !m_RightButtonPressed)
	{
		app->CycleModelRight();
		m_RightButtonPressed = true;
	}
	else if (glfwGetKey(window, m_Keys.cycleModelRight) == GLFW_RELEASE && m_RightButtonPressed)
		m_RightButtonPressed = false;

}