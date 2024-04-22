#include "window.h"

#include <stdexcept>

FH::FHWindow::FHWindow(int width, int height, const std::string& name)
	: m_Width{ width }
	, m_Height{ height }
	, m_WindowName{ name }
{
	InitWindow();
}

FH::FHWindow::~FHWindow()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void FH::FHWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		throw std::runtime_error("failed to create window surface");
}

void FH::FHWindow::InitWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
}

void FH::FHWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto updatedWindow{ reinterpret_cast<FHWindow*>(glfwGetWindowUserPointer(window)) };
	updatedWindow->m_FramebufferResized = true;
	updatedWindow->m_Width = width;
	updatedWindow->m_Height = height;
}