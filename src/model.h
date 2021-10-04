#pragma once

#include "stdinclude.h"

#include "device.h"
#include "tools.h"

class Model{
public:
    Model(Device* device)
    {
        this->device = device;
    }
    ~Model()
    {
        if(indices.size() > 0)
        {
            vkDestroyBuffer(device->device, indexBuffer, nullptr);
            vkFreeMemory(device->device, indexBufferMemory, nullptr);
        }
        
        if(vertices.size() > 0)
        {
            vkDestroyBuffer(device->device, vertexBuffer, nullptr);
            vkFreeMemory(device->device, vertexBufferMemory, nullptr);
        }
    }
    void Init()
    {
        createVertexBuffer();
        createIndexBuffer();
    }

    void createVertexBuffer()
    {
        if(vertices.size() == 0) return;

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Tools::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(device->device, stagingBufferMemory);

        Tools::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        Tools::copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device->device, stagingBuffer, nullptr);
        vkFreeMemory(device->device, stagingBufferMemory, nullptr);
    }
    
    void createIndexBuffer() {

        if(indices.size() == 0) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        Tools::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device->device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(device->device, stagingBufferMemory);

        Tools::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        Tools::copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device->device, stagingBuffer, nullptr);
        vkFreeMemory(device->device, stagingBufferMemory, nullptr);

    }

    std::vector<Vertex> vertices;

    std::vector<uint16_t> indices;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;

    Device* device;

};