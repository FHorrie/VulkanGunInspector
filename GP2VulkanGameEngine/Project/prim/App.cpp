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
    LoadGameObjects();
    //LoadGameObjects2D();

    m_pAppPool = FHDescriptorPool::Builder(m_FHDevice)
        //"." chaining (See descriptor pool builder declaration!!!)
        .SetMaxSets(FHSwapChain::MAX_FRAMES_IN_FLIGHT + 
            FHSwapChain::MAX_FRAMES_IN_FLIGHT * static_cast<int>(m_Models.size()))

        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FHSwapChain::MAX_FRAMES_IN_FLIGHT)

        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
            FHSwapChain::MAX_FRAMES_IN_FLIGHT * static_cast<int>(m_Models.size()))

        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)

        .Build();

    PrintControls();
}

void FH::FirstApp::Run()
{
    ////////////////////////
    // UNIFORM BUFFER LOGIC
    ////////////////////////

    std::vector<std::unique_ptr<FHBuffer>> globalUboBuffers(FHSwapChain::MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSet> appDescriptorSets(FHSwapChain::MAX_FRAMES_IN_FLIGHT);

    auto globalSetLayout
    {
        FHDescriptorSetLayout::Builder(m_FHDevice)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .Build()
    };

    auto objectSetLayout
    {
        FHDescriptorSetLayout::Builder(m_FHDevice)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build()
    };

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts =
    {
        globalSetLayout->GetDescriptorSetLayout(),
        objectSetLayout->GetDescriptorSetLayout()
    };

    auto diffusePlaceHolder{ std::make_unique<FHTexture>(m_FHDevice, "textures/MissingTex.png") };
    
    for (int i{}; i < FHSwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        globalUboBuffers[i] = std::make_unique<FHBuffer>(
            m_FHDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        globalUboBuffers[i]->Map();
    
        auto bufferInfo{ globalUboBuffers[i]->GetDescriptorInfo() };
        FHDescriptorWriter(*globalSetLayout, *m_pAppPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(appDescriptorSets[i]);

        for (int meshIdx{}; meshIdx < static_cast<int>(m_Models.size()); ++meshIdx)
        {
            auto currentObj = m_Models[meshIdx].get();

            VkDescriptorImageInfo imageDiffuseInfo{};
            if (m_Models[meshIdx]->m_DiffuseTexture)
                imageDiffuseInfo = {
                    currentObj->m_DiffuseTexture->GetTextureSampler(),
                    currentObj->m_DiffuseTexture->GetTextureImageView(),
                    currentObj->m_DiffuseTexture->GetTextureImageLayout() };
            else
                imageDiffuseInfo = {
                    diffusePlaceHolder->GetTextureSampler(),
                    diffusePlaceHolder->GetTextureImageView(),
                    diffusePlaceHolder->GetTextureImageLayout() };

            VkDescriptorImageInfo imageNormalInfo{};
            if (m_Models[meshIdx]->m_NormalTexture)
                imageNormalInfo = {
                    currentObj->m_NormalTexture->GetTextureSampler(),
                    currentObj->m_NormalTexture->GetTextureImageView(),
                    currentObj->m_NormalTexture->GetTextureImageLayout() };

            VkDescriptorImageInfo imageRoughnessInfo{};
            if (m_Models[meshIdx]->m_RoughnessTexture)
                imageRoughnessInfo = {
                    currentObj->m_RoughnessTexture->GetTextureSampler(),
                    currentObj->m_RoughnessTexture->GetTextureImageView(),
                    currentObj->m_RoughnessTexture->GetTextureImageLayout() };

            VkDescriptorImageInfo imageMetallicInfo{};
            if (m_Models[meshIdx]->m_MetallicTexture)
                imageMetallicInfo = {
                    currentObj->m_MetallicTexture->GetTextureSampler(),
                    currentObj->m_MetallicTexture->GetTextureImageView(),
                    currentObj->m_MetallicTexture->GetTextureImageLayout() };

            VkDescriptorImageInfo imageAOInfo{};
            if (m_Models[meshIdx]->m_AOTexture)
                imageAOInfo = {
                    currentObj->m_AOTexture->GetTextureSampler(),
                    currentObj->m_AOTexture->GetTextureImageView(),
                    currentObj->m_AOTexture->GetTextureImageLayout() };
            
            VkDescriptorSet descriptorSet{};
            FHDescriptorWriter(*objectSetLayout, *m_pAppPool)
                .WriteImage(0, &imageDiffuseInfo)
                .WriteImage(1, &imageNormalInfo)
                .WriteImage(2, &imageRoughnessInfo)
                .WriteImage(3, &imageMetallicInfo)
                .WriteImage(4, &imageAOInfo)
                .Build(descriptorSet);

            currentObj->SetDescriptorSetAtFrame(i, descriptorSet);
        }

    }
    ////////////////////////

    FHRenderSystem renderSystem
    {
        m_FHDevice,
        m_FHRenderer.GetSwapChainRenderPass(),
        descriptorSetLayouts
    };
    
    FHCamera camera{};

    auto viewerObject = FHGameObject::CreateGameObject();
    KeyboardInput cameraController{};

    while (!m_FHWindow.ShouldClose())
    {
        glfwPollEvents();

        Time::UpdateTime();

        const float aspect = m_FHRenderer.GetAspectRatio();
        cameraController.MoveInPlaneXZ(m_FHWindow.GetWindowPointer(), viewerObject, this);
        camera.SetViewYXZ(viewerObject.m_Transform.translation, viewerObject.m_Transform.rotation);
        camera.SetPerspectiveProjection(glm::radians(45.f), aspect, 0.3f, 100.f);

        if (auto commandBuffer = m_FHRenderer.BeginFrame()) //nullptr when swapchain needs to be remade
        {
            const int frameIdx{ m_FHRenderer.GetFrameIndex() };

            FHFrameInfo frameInfo{ 
                frameIdx, 
                commandBuffer, 
                camera, 
                appDescriptorSets[frameIdx]
            };

            //update
            GlobalUbo ubo{};
            ubo.m_Projection = camera.GetProjectionMatrix();
            ubo.m_View = camera.GetViewMatrix();

            int dirLightIndex{};
            for (const auto& light : m_DirLights)
            {
                if (light->m_DirLightComp != nullptr)
                {
                    ubo.m_DirectionalLights[dirLightIndex].m_Direction 
                        = glm::vec4(light->m_DirLightComp->direction, 1.f);

                    ubo.m_DirectionalLights[dirLightIndex].m_Color 
                        = glm::vec4(light->m_Color, light->m_DirLightComp->lightIntensity);

                    ++dirLightIndex;
                }
            }
            ubo.m_DirectionalLightAmount = dirLightIndex;

            globalUboBuffers[frameIdx]->WriteToBuffer(&ubo);
            globalUboBuffers[frameIdx]->FlushBufferMemRange();

            if (m_ModelRotate)
                for (int idx{}; idx < static_cast<int>(m_Models.size()); ++idx)
                {
                    m_Models[idx]->m_Transform.rotation.y +=
                    glm::radians(30.f) * Time::GetDeltaTime();

                    if (m_Models[idx]->m_Transform.rotation.y > 360.f)
                    {
                        m_Models[idx]->m_Transform.rotation.y -= 360.f;
                    }
                }

            //render
            m_FHRenderer.BeginSwapChainRenderPass(commandBuffer);

            std::vector<FHGameObject*> pModelVec{};
            for (int modelIdx{}; modelIdx < static_cast<int>(m_Models.size()); ++modelIdx)
                pModelVec.push_back(m_Models[modelIdx].get());

            renderSystem.RenderGameObjects(frameInfo, pModelVec);
            //renderSystem.RenderGameObjects2D(frameInfo, m_GameObjects2D);
            
            m_FHRenderer.EndSwapChainRenderPass(commandBuffer);
            m_FHRenderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_FHDevice.GetDevice());
}



void FH::FirstApp::LoadGameObjects()
{
    std::unique_ptr<FHModel> deagleModel = FHModel::CreateModelFromFile(m_FHDevice,
        "models/deagle.obj");

    auto deagle = std::make_unique<FHGameObject>(FHGameObject::CreateGameObject());

    deagle->m_Model = std::move(deagleModel);
    deagle->m_Transform.translation = { 0.f, 3.5f, 2.5f };
    deagle->m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    deagle->m_Transform.rotation = { 0.f, 0.f, 0.f };

    auto deagleDiffuseMap = std::make_unique<FHTexture>(m_FHDevice, "textures/deagle/deagle_diffuse.png");
    auto deagleNormalMap = std::make_unique<FHTexture>(m_FHDevice, "textures/deagle/deagle_normal.png");
    auto deagleRoughnessMap = std::make_unique<FHTexture>(m_FHDevice, "textures/deagle/deagle_roughness.png");
    auto deagleMetallicMap = std::make_unique<FHTexture>(m_FHDevice, "textures/deagle/deagle_metallic.png");
    auto deagleAOMap = std::make_unique<FHTexture>(m_FHDevice, "textures/deagle/deagle_ao.png");

    deagle->m_DiffuseTexture = std::move(deagleDiffuseMap);
    deagle->m_NormalTexture = std::move(deagleNormalMap);
    deagle->m_RoughnessTexture = std::move(deagleRoughnessMap);
    deagle->m_MetallicTexture = std::move(deagleMetallicMap);
    deagle->m_AOTexture = std::move(deagleAOMap);

    m_Models.push_back(std::move(deagle));

    std::unique_ptr<FHModel> akModel = FHModel::CreateModelFromFile(m_FHDevice,
        "models/ak47.obj");

    auto ak47 = std::make_unique<FHGameObject>(FHGameObject::CreateGameObject());
    ak47->m_Model = std::move(akModel);
    ak47->m_Transform.translation = { 0.f, 0.5f, 2.5f };
    ak47->m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    ak47->m_Transform.rotation = { 0.f, 0.f, 0.f };

    auto ak47DiffuseMap = std::make_unique<FHTexture>(m_FHDevice, "textures/ak47/ak47_diffuse.png");
    auto ak47NormalMap = std::make_unique<FHTexture>(m_FHDevice, "textures/ak47/ak47_normal.png");
    auto ak47RoughnessMap = std::make_unique<FHTexture>(m_FHDevice, "textures/ak47/ak47_roughness.png");
    auto ak47MetallicMap = std::make_unique<FHTexture>(m_FHDevice, "textures/ak47/ak47_metallic.png");
    auto ak47AOMap = std::make_unique<FHTexture>(m_FHDevice, "textures/ak47/ak47_ao.png");

    ak47->m_DiffuseTexture = std::move(ak47DiffuseMap);
    ak47->m_NormalTexture = std::move(ak47NormalMap);
    ak47->m_RoughnessTexture = std::move(ak47RoughnessMap);
    ak47->m_MetallicTexture = std::move(ak47MetallicMap);
    ak47->m_AOTexture = std::move(ak47AOMap);

    m_Models.push_back(std::move(ak47));

    std::unique_ptr<FHModel> m4a4Model = FHModel::CreateModelFromFile(m_FHDevice,
        "models/m4a4.obj");

    auto m4a4 = std::make_unique<FHGameObject>(FHGameObject::CreateGameObject());

    m4a4->m_Model = std::move(m4a4Model);
    m4a4->m_Transform.translation = { 0.f, -2.5f, 2.5f };
    m4a4->m_Transform.scale = { 0.25f, 0.25f, 0.25f };
    m4a4->m_Transform.rotation = { 0.f, 0.f, 0.f };

    auto m4a4DiffuseMap = std::make_unique<FHTexture>(m_FHDevice, "textures/m4a4/m4a4_diffuse.png");
    auto m4a4NormalMap = std::make_unique<FHTexture>(m_FHDevice, "textures/m4a4/m4a4_normal.png");
    auto m4a4RoughnessMap = std::make_unique<FHTexture>(m_FHDevice, "textures/m4a4/m4a4_roughness.png");
    auto m4a4MetallicMap = std::make_unique<FHTexture>(m_FHDevice, "textures/m4a4/m4a4_metallic.png");
    auto m4a4AOMap = std::make_unique<FHTexture>(m_FHDevice, "textures/m4a4/m4a4_ao.png");

    m4a4->m_DiffuseTexture = std::move(m4a4DiffuseMap);
    m4a4->m_NormalTexture = std::move(m4a4NormalMap);
    m4a4->m_RoughnessTexture = std::move(m4a4RoughnessMap);
    m4a4->m_MetallicTexture = std::move(m4a4MetallicMap);
    m4a4->m_AOTexture = std::move(m4a4AOMap);

    m_Models.push_back(std::move(m4a4));

    // Fix .obj export orientation
    for (int idx{}; idx < static_cast<int>(m_Models.size()); ++idx)
    {
        m_Models[idx]->m_Transform.rotation.y += glm::radians(90.f);
        m_Models[idx]->m_Transform.rotation.z += glm::radians(180.f);
    }

    //Add light
    auto mainLight = std::make_unique<FHGameObject>(FHGameObject::CreateDirectionalLight());

    m_DirLights.push_back(std::move(mainLight));
}

//void FH::FirstApp::LoadGameObjects2D()
//{
//    std::vector<FHModel2D::Vertex2D> vertices
//    {
//        {{0.0f, -0.5f}, {1.f, 0.f, 0.f}},
//        {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
//        {{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}
//    };
//    auto model = std::make_shared<FHModel2D>(m_FHDevice, vertices);
//
//    std::vector<glm::vec3> colors{
//      {1.f, .7f, .73f},
//      {1.f, .87f, .73f},
//      {1.f, 1.f, .73f},
//      {.73f, 1.f, .8f},
//      {.73, .88f, 1.f}  //
//    };
//    for (auto& color : colors) {
//        color = glm::pow(color, glm::vec3{ 2.2f });
//    }
//
//    for (int i = 0; i < 40; i++) {
//        auto triangle = FHGameObject2D::CreateGameObject();
//        triangle.m_Model = model;
//        //triangle.m_Transform2D.translation.x = 0.2f;
//        triangle.m_Transform2D.translation = glm::vec2{ 0.8f, -0.8f };
//        triangle.m_Transform2D.scale = glm::vec2{ 0.5f } + i * 0.025f;
//        triangle.m_Transform2D.rotation = i * glm::pi<float>() * 0.025f;
//        triangle.m_Color = colors[i % colors.size()];
//        m_GameObjects2D.push_back(std::move(triangle));
//    }
//}

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