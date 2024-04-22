#include "App.h"
#include "engine/renderSystem.h"
#include "camera.h"
#include "engine/FHTime.h"
#include "engine/keyboardInput.h"
#include "engine/frameInfo.h"

#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <numeric>
#include <iostream>

FH::FirstApp::FirstApp()
{
    m_pAppPool = FHDescriptorPool::Builder(m_FHDevice)
        //"." chaining (See descriptor pool builder declaration!!!)
        .SetMaxSets(FHSwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FHSwapChain::MAX_FRAMES_IN_FLIGHT)
        .Build();

	LoadGameObjects();
	LoadGameObjects2D();

    PrintControls();
}

void FH::FirstApp::Run()
{
    ////////////////////////
    // UNIFORM BUFFER LOGIC
    ////////////////////////
    std::vector<std::unique_ptr<FHBuffer>> globalUniBuffers(FHSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i{}; i < static_cast<int>(globalUniBuffers.size()); i++)
    {
        globalUniBuffers[i] = std::make_unique<FHBuffer>(
            m_FHDevice,
            sizeof(GlobalUniBuffer),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        globalUniBuffers[i]->Map();
    }

    std::vector<std::unique_ptr<FHBuffer>> textureUniBuffers(FHSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i{}; i < static_cast<int>(textureUniBuffers.size()); i++)
    {
        textureUniBuffers[i] = std::make_unique<FHBuffer>(
            m_FHDevice,
            sizeof(MaterialUniBuffer),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        globalUniBuffers[i]->Map();
    }

    auto appSetLayout
    {
        FHDescriptorSetLayout::Builder(m_FHDevice)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .AddBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .Build()
    };

    std::vector<VkDescriptorSet> appDescriptorSets(FHSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i{}; i < static_cast<int>(appDescriptorSets.size()); ++i)
    {
        auto bufferInfo{ globalUniBuffers[i]->GetDescriptorInfo() };
        FHDescriptorWriter(*appSetLayout, *m_pAppPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(appDescriptorSets[i]);

    }
    ////////////////////////

    FHRenderSystem renderSystem{ 
        m_FHDevice, m_FHRenderer.GetSwapChainRenderPass(), appSetLayout->GetDescriptorSetLayout() };
    FHCamera camera{};

    auto viewerObject = FHGameObject::CreateGameObject();
    KeyboardInput cameraController{};

	while (!m_FHWindow.ShouldClose())
	{
		glfwPollEvents();
        
        Time::UpdateTime();

        cameraController.MoveInPlaneXZ(m_FHWindow.GetWindowPointer(), viewerObject, this);
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation);
        
        float aspect = m_FHRenderer.GetAspectRatio();
        camera.SetPerspectiveProjection(glm::radians(45.f), aspect, 0.3f, 100.f);
        if (auto commandBuffer = m_FHRenderer.BeginFrame()) //nullptr when swapchain needs to be remade
		{
            const int frameIdx{ m_FHRenderer.GetFrameIndex() };
            FHFrameInfo frameInfo{ frameIdx, commandBuffer, camera, appDescriptorSets[frameIdx]};

            //update
            GlobalUniBuffer ubo{};
            ubo.m_ProjectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
            globalUniBuffers[frameIdx]->WriteToBuffer(&ubo);
            globalUniBuffers[frameIdx]->FlushBufferMemRange();

            if(m_ModelRotate)
                for (int idx{}; idx < static_cast<int>(m_GameObjects.size()); ++idx)
                    m_GameObjects[idx].m_Transform.rotation.y += 
                    glm::radians(30.f) * Time::GetDeltaTime();

            //render
			m_FHRenderer.BeginSwapChainRenderPass(commandBuffer);
            renderSystem.RenderGameObjects(frameInfo, m_GameObjects);
            //renderSystem.RenderGameObjects2D(frameInfo, m_GameObjects2D);
			m_FHRenderer.EndSwapChainRenderPass(commandBuffer);
			m_FHRenderer.EndFrame();
		}
	}

	vkDeviceWaitIdle(m_FHDevice.GetDevice());
}



void FH::FirstApp::LoadGameObjects()
{
    std::shared_ptr<FHModel> deagleModel = FHModel::CreateModelFromFile(m_FHDevice, 
        "resources/models/deagle.obj");

    auto deagle = FHGameObject::CreateGameObject();
    deagle.m_Model = deagleModel;
    deagle.m_Transform.translation = { 0.f, 3.5f, 2.5f };
    deagle.m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    deagle.m_Transform.rotation = { 0.f, 0.f, 0.f };
    m_GameObjects.push_back(std::move(deagle));

    std::shared_ptr<FHModel> akModel = FHModel::CreateModelFromFile(m_FHDevice,
        "resources/models/ak47.obj");

    auto ak47 = FHGameObject::CreateGameObject();
    ak47.m_Model = akModel;
    ak47.m_Transform.translation = { 0.f, 0.5f, 2.5f };
    ak47.m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    ak47.m_Transform.rotation = { 0.f, 0.f, 0.f };
    m_GameObjects.push_back(std::move(ak47));

    std::shared_ptr<FHModel> m4a1Model = FHModel::CreateModelFromFile(m_FHDevice,
        "resources/models/m4a1_silencer.obj");

    auto m4a1 = FHGameObject::CreateGameObject();
    m4a1.m_Model = m4a1Model;
    m4a1.m_Transform.translation = { 0.f, -2.5f, 2.5f };
    m4a1.m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    m4a1.m_Transform.rotation = { 0.f, 0.f, 0.f };
    m_GameObjects.push_back(std::move(m4a1));

    // Fix .obj export orientation
    for (int idx{}; idx < static_cast<int>(m_GameObjects.size()); ++idx)
    {
        m_GameObjects[idx].m_Transform.rotation.y += glm::radians(90.f);
        m_GameObjects[idx].m_Transform.rotation.z += glm::radians(180.f);
    }
}

void FH::FirstApp::LoadGameObjects2D()
{
    std::vector<FHModel2D::Vertex> vertices
    {
        {{0.0f, -0.5f}, {1.f, 0.f, 0.f}},
        {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
        {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}
    };
    auto model = std::make_shared<FHModel2D>(m_FHDevice, vertices);

    std::vector<glm::vec3> colors{
      {1.f, .7f, .73f},
      {1.f, .87f, .73f},
      {1.f, 1.f, .73f},
      {.73f, 1.f, .8f},
      {.73, .88f, 1.f}  //
    };
    for (auto& color : colors) {
        color = glm::pow(color, glm::vec3{ 2.2f });
    }

    for (int i = 0; i < 40; i++) {
        auto triangle = FHGameObject2D::CreateGameObject();
        triangle.m_Model = model;
        //triangle.m_Transform2D.translation.x = 0.2f;
        triangle.m_Transform2D.translation = glm::vec2{ 0.8f, -0.8f };
        triangle.m_Transform2D.scale = glm::vec2{ 0.5f } + i * 0.025f;
        triangle.m_Transform2D.rotation = i * glm::pi<float>() * 0.025f;
        triangle.m_Color = colors[i % colors.size()];
        m_GameObjects2D.push_back(std::move(triangle));
    }
}

void FH::FirstApp::PrintControls()
{
    std::cout << "\n----------------------------\n";
    std::cout << "-- Controls:\n";
    std::cout << "-- WASD -> Movement\n";
    std::cout << "-- RMB + Mouse -> Aiming\n";
    std::cout << "-- LMB -> Speed up movement\n";
    std::cout << "-- F5 -> Enable model rotation\n";
    std::cout << "----------------------------\n\n";
}