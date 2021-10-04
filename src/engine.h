#pragma once

#include "stdinclude.h"
#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "graphics.h"

#include "game.h"

#include "resource.h"

class Engine{
public:
    Engine(){}
    virtual ~Engine(){

        
        for (size_t i = 0; i < Resource::countFrames; i++) {
            vkDestroySemaphore(device->device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device->device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device->device, inFlightFences[i], nullptr);
        }
                
        delete game;
        delete graphics;
        delete swapchain;
        //ImGui_ImplVulkanH_DestroyWindow(window->instance, device->device, &Resource::g_MainWindowData, window->g_Allocator);
        delete device;
        delete window;
        
        vkDestroyDescriptorPool(device->device, imguiPool, nullptr);       
    }

    public:
    void Init()
    {
        window = new WindowManager();
        window->Init();

        device = new Device(window);
        device->Init();
        //device->setupImGUI();

        Tools::device = device;

        swapchain = new SwapChain(window, device);
        swapchain->Init();

        graphics = new Graphics(window, device, swapchain);

        game = new Game(device, graphics);
        game->Init();
        
        GUIInit();

        graphics->Init();

        createSyncObjects();

    }
    void Run() {
        graphics->setCommandBuffers();
        while (!window->GetClose()) {
            glfwPollEvents();

            draw();

            if(Resource::pressed[GLFW_KEY_ESCAPE])
                glfwSetWindowShouldClose(window->window, GL_TRUE);
        }
        
        vkDeviceWaitIdle(device->device);

        
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void GUIInit(){

        // Create Descriptor Pool
        
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
            
        vkCreateDescriptorPool(device->device, &pool_info, nullptr, &imguiPool);
        
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(window->window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = window->instance;
        init_info.PhysicalDevice = device->physicalDevice;
        init_info.Device = device->device;
        init_info.QueueFamily = device->g_QueueFamily;
        init_info.Queue = device->graphicsQueue;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 3;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&init_info, graphics->renderer->renderPass);

        {
            // Use any command queue
            VkCommandBuffer command_buffer = Tools::beginSingleTimeCommands();

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            Tools::endSingleTimeCommands(command_buffer);
            
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
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

        game->Update(time);

        if(Resource::pressed[GLFW_KEY_LEFT_CONTROL] && !menuSwaped)  
        { 
            Resource::showCursor = !Resource::showCursor;
            menuSwaped = true;
        }

        
        if(!Resource::pressed[GLFW_KEY_LEFT_CONTROL])
            menuSwaped = false;   

        if(!Resource::showCursor)     
            glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    }

    void draw()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show);

        ImGui::Render();    

        uint32_t imageIndex;       

        FrameRender(&imageIndex);
        FramePresent(imageIndex);
        
    }

    void FrameRender(uint32_t* imageIndex)
    {
        VkResult err;
         

        vkWaitForFences(device->device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);


        err = vkAcquireNextImageKHR(device->device, swapchain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, imageIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        
        Resource::currentImage = currentFrame;

        Update();

        // Check if a previous frame is using this image (i.e. there is its fence to wait on)
        if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device->device, 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        
        // Mark the image as now being in use by this frame
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];   
        
        MakeFrame();

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = &wait_stage;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &graphics->commandBuffers[*imageIndex];
        
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device->device, 1, &inFlightFences[currentFrame]);
        
        vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

        vkQueueWaitIdle(device->graphicsQueue);
        
    }

    void MakeFrame()
    {

        vkResetCommandPool(device->device, Resource::commandPool, 0);
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(graphics->commandBuffers[currentFrame], &beginInfo);
            
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = graphics->renderer->renderPass;
        info.framebuffer = graphics->swapChainFramebuffers[currentFrame];
        info.renderArea.extent.width = Resource::swapChainExtent.width;
        info.renderArea.extent.height =  Resource::swapChainExtent.height;
        info.clearValueCount = static_cast<uint32_t>(clearValues.size());
        info.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(graphics->commandBuffers[currentFrame], &info, VK_SUBPASS_CONTENTS_INLINE);
            
        game->Draw(graphics->commandBuffers[currentFrame], currentFrame);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), graphics->commandBuffers[currentFrame]);

        // Submit command buffer
        vkCmdEndRenderPass(graphics->commandBuffers[currentFrame]);

        vkEndCommandBuffer(graphics->commandBuffers[currentFrame]);

    }

    void FramePresent(uint32_t imageIndex)
    {
        if (g_SwapChainRebuild)
            return;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

        VkSwapchainKHR swapChains[] = {swapchain->swapChain};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        VkResult err = vkQueuePresentKHR(device->presentQueue, &presentInfo);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        
        vkQueueWaitIdle(device->presentQueue);

        currentFrame = (currentFrame + 1) % Resource::countFrames;

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

        currentFrame = (currentFrame + 1) % Resource::countFrames;
    }

    void createSyncObjects() {        
        imageAvailableSemaphores.resize(Resource::countFrames);
        renderFinishedSemaphores.resize(Resource::countFrames);
        inFlightFences.resize(Resource::countFrames);
        imagesInFlight.resize(Resource::countFrames, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < Resource::countFrames; i++) {
            if (vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }


    VkPipelineCache g_PipelineCache = VK_NULL_HANDLE;

    bool show_demo_window = true;

    bool g_SwapChainRebuild = false;

    Game* game;

    bool menuSwaped = false;

    bool show = true;
	    
    VkDescriptorPool imguiPool;
};