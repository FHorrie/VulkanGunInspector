#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace FH
{
	class FHWindow
	{
	public:

		FHWindow(int width, int height, const std::string& windowName);
		~FHWindow();

		FHWindow(const FHWindow&) = delete;
		FHWindow& operator=(const FHWindow&) = delete;

		inline bool ShouldClose() 
		{ 
			return glfwWindowShouldClose(m_Window);
		}
		VkExtent2D GetExtent() 
		{ 
			return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; 
		}

		GLFWwindow* GetWindowPointer() const { return m_Window; }
		bool IsWindowResized() const { return m_FramebufferResized; }
		void ResetWindowResizedFlag() { m_FramebufferResized = false; }

		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
		void InitWindow();

		int m_Width;
		int m_Height;
		bool m_FramebufferResized{ false };

		std::string m_WindowName;
		GLFWwindow* m_Window;
	};
}