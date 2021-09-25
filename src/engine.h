#pragma once

#include "stdinclude.h"
#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "graphics.h"

#include "camera.h"
#include "skyBox.h"
#include "gameObject.h"

#include "resource.h"

class Engine{
public:
    Engine(){}
    virtual ~Engine(){

        delete gameObject;
        delete gameObject2;
        delete skyBox;
        
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device->device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device->device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device->device, inFlightFences[i], nullptr);
        }
        
        delete graphics;
        delete swapchain;
        delete device;
        delete window;
    }

    public:
    void Init()
    {
        window = new WindowManager();
        window->Init();

        device = new Device(window);
        device->Init();

        Tools::device = device;

        swapchain = new SwapChain(window, device);
        swapchain->Init();

        graphics = new Graphics(window, device, swapchain);

        
        camera = new Camera(Resource::swapChainExtent.width, Resource::swapChainExtent.height);
        
        PrimitiveObject pObject = Tools::GetPrimitives(PrimitiveType::PRIMITIVE_TYPE_CAPSULE);

        gameObject = new GameObject(device, camera);
        gameObject->SetShadersName("shaders/vert.spv","shaders/frag.spv");    
        gameObject->SetSize(2);      
        gameObject->SetPosition({0,0,60});  
        gameObject->setVertex(pObject.vertices);
        gameObject->setIndices(pObject.indices);

        gameObject2 = new GameObject(device, camera);      
        gameObject2->SetSize(2);
        gameObject2->SetShadersName("shaders/vert.spv","shaders/frag2.spv");
        gameObject2->setVertex(pObject.vertices);
        gameObject2->setIndices(pObject.indices);

        
        skyBox = new SkyBox(device, camera, {"shaders/vert.spv","shaders/frag.spv"});

        graphics->SetGameObject(gameObject);
        graphics->SetGameObject(gameObject2);
        graphics->SetGameObject(skyBox->go);

        graphics->Init();

        createSyncObjects();
    }
    void Run() {
        while (!window->GetClose()) {
            glfwPollEvents();

            drawFrame();

            if(Resource::pressed[GLFW_KEY_ESCAPE])
                glfwSetWindowShouldClose(window->window, GL_TRUE);
        }
        
        vkDeviceWaitIdle(device->device);
    }

private:
    WindowManager* window;
    Device* device;
    SwapChain* swapchain;
    Graphics* graphics;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    
    void Update()
    {        
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() / 1000;

        camera->Update(time);
        gameObject->SetRotate({0,0,10});
        gameObject->Update(time);
        gameObject2->Update(time);
        skyBox->Update(time);
    }

    void drawFrame() {
        vkWaitForFences(device->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device->device, swapchain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        Resource::currentImage = currentFrame;

        Update();
        
        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device->device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(graphics->commandBuffers[imageIndex]);

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device->device, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapchain->swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(device->presentQueue, &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void createSyncObjects() {        
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapchain->swapChainImages.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
    
    Camera* camera;

    GameObject* gameObject;
    GameObject* gameObject2;
    SkyBox* skyBox;
};