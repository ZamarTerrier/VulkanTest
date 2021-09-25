#pragma once

#include "stdinclude.h"

#include "resource.h"

#include "device.h"
#include "tools.h"
#include "window.h"
#include "swapchain.h"
#include "renderer.h"

#include "gameObject.h"

class Graphics{
public:
    Graphics(WindowManager *window, Device* device, SwapChain* swapchain)
    {
        this->window = window;
        this->device = device;
        this->swapchain = swapchain;

        renderer = new Renderer(device);        

    }
    ~Graphics()
    {
        delete renderer;

        vkDestroyImageView(device->device, depthImageView, nullptr);

        vkDestroyImage(device->device, depthImage, nullptr);
        vkFreeMemory(device->device, depthImageMemory, nullptr);
        
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device->device, framebuffer, nullptr);
        }
    }

    void SetGameObject(GameObject* go)
    {
        gameObjects.push_back(go);
    }

    void Init(){        
        createDepthResources();
        createFramebuffers(); 
        for(auto go : gameObjects)
        {
            renderer->createGraphicsPipeline(go->vertFile, go->fragFile, go);  
            go->Init();
        }
        createCommandBuffers();
        setCommandBuffers();
    }
    
    void createFramebuffers() {
        swapChainFramebuffers.resize(swapchain->swapChainImageViews.size());
        for (size_t i = 0; i < swapchain->swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapchain->swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderer->renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = Resource::swapChainExtent.width;
            framebufferInfo.height = Resource::swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device->device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = renderer->findDepthFormat();

        Tools::createImage(Resource::swapChainExtent.width, Resource::swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = Tools::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }


    void createCommandBuffers() {
        commandBuffers.resize(swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = Resource::commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device->device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }       
        
    }

    void setCommandBuffers()
    {
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderer->renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = Resource::swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());;
            renderPassInfo.pClearValues = clearValues.data();
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            for(auto go : gameObjects)
            {
                vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, go->graphicsPipeline);

                go->Draw(commandBuffers[i], i);
            }
                       
            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    WindowManager* window;
    Device* device;
    SwapChain* swapchain;
    Renderer* renderer;

    std::vector<GameObject*> gameObjects;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};