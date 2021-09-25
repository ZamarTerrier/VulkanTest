#pragma once

#include "device.h"
#include "resource.h"

struct PrimitiveObject{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

enum PrimitiveType{
    PRIMITIVE_TYPE_SPHERE,
    PRIMITIVE_TYPE_CILINDER,
    PRIMITIVE_TYPE_CUBE,
    PRIMITIVE_TYPE_CAPSULE,    
    PRIMITIVE_TYPE_QUAD
};

class Tools{
public:
    Tools(Device* device)
    {
        this->device = device;
    }
    ~Tools()
    {

    }

    static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        

        VkImageView imageView;
        if (vkCreateImageView(device->device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device->device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device->device, image, imageMemory, 0);
    }

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device->device, buffer, bufferMemory, 0);
    }

    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    static void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer);
    }

    static VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = Resource::commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    static void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device->graphicsQueue);

        vkFreeCommandBuffers(device->device, Resource::commandPool, 1, &commandBuffer);
    }

    static PrimitiveObject GetPrimitives(PrimitiveType type)
    {
        switch(type)
        {
            case PrimitiveType::PRIMITIVE_TYPE_SPHERE:
                return CreateSphere();
            case PrimitiveType::PRIMITIVE_TYPE_CILINDER:
                return CreatePipe();
            case PrimitiveType::PRIMITIVE_TYPE_CUBE:
                return box;
            case PrimitiveType::PRIMITIVE_TYPE_CAPSULE:
                return CreateCapsule();
            case PrimitiveType::PRIMITIVE_TYPE_QUAD:
                return quad;
        }

        return PrimitiveObject({{},{}});
    }

    inline static Device* device;

private:

    static PrimitiveObject CreateSphere()
    {
        PrimitiveObject pObject;

        float x, y, z, xy;   
        float radius = 0.2f;                           // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        int stackCount = 20;
        int sectorCount = 20;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for(int i = 0; i <= stackCount; ++i)
        {
            stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for(int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                
                pObject.vertices.push_back({{x,y,z}, {s,t,t}});
            }
        }

        int k1, k2;
        for(int i = 0; i < stackCount; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if(i != 0)
                {
                    pObject.indices.push_back(k1 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1);
                }

                // k1+1 => k2 => k2+1
                if(i != (stackCount-1))
                {
                    pObject.indices.push_back(k2 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1 + 1);
                }
            }
        }

        return pObject;
    }

    static PrimitiveObject GetUnitForCilinder(int sectCount)
    {
        PrimitiveObject pObject;

        float sectorStep = 2 * M_PI / sectCount;
        float sectorAngle;  // radian

        std::vector<float> unitCircleVertices;
        for(int i = 0; i <= sectCount; ++i)
        {
            sectorAngle = i * sectorStep;
            pObject.vertices.push_back({{cos(sectorAngle), sin(sectorAngle), 0},{cos(sectorAngle),sin(sectorAngle),1.0f}});
        }     

        return pObject;
    }

    static PrimitiveObject CreatePipe(){
        
        PrimitiveObject pObject;        

        float radius = 1;
        float height = 3;
        int sectorCount = 10;

        pObject = GetUnitForCilinder(sectorCount);

        // put side vertices to arrays
        for(int i = 0; i < 2; ++i)
        {
            float h = -height + i * height;           // z value; -h/2 to h/2
            float t = 1.0f - i;                              // vertical tex coord; 1 to 0

            for(int j = 0, k = 0; j <= sectorCount; ++j, k ++)
            {
                float ux = pObject.vertices[k].pos.x;
                float uy = pObject.vertices[k].pos.y;
                float uz = pObject.vertices[k].pos.z;

                pObject.vertices.push_back({{ux * radius, uy * radius, h},{(float)j / sectorCount, t , t}}); 
            }
        }

        // the starting index for the base/top surface
        //NOTE: it is used for generating indices later
        int baseCenterIndex = pObject.vertices.size();
        int topCenterIndex =  baseCenterIndex + sectorCount + 1; // include center vertex

        // put base and top vertices to arrays
        for(int i = 0; i < 2; ++i)
        {
            float h = -height + i * height;           // z value; -h/2 to h/2
            float nz = -1 + i * 2;                           // z value of normal; -1 to 1
 
            pObject.vertices.push_back({{0, 0, h},{0.5f, 0.5f , 0.5f}}); 
            
            for(int j = 0, k = 0; j < sectorCount; ++j, k ++)
            {
                float ux = pObject.vertices[k].pos.x;
                float uy = pObject.vertices[k].pos.y;
                pObject.vertices.push_back({{ux * radius, uy * radius, h},{-ux * 0.5f + 0.5f, -uy * 0.5f + 0.5f, -uy * 0.5f + 0.5f}}); 
            }
        }
        
        int k1 = 0;                         // 1st vertex index at base
        int k2 = sectorCount + 1;           // 1st vertex index at top

        // indices for the side surface
        for(int i = 0; i < sectorCount + 1; ++i, ++k1, ++k2)
        {
            // 2 triangles per sector
            // k1 => k1+1 => k2
            pObject.indices.push_back(k1);
            pObject.indices.push_back(k1 + 1);
            pObject.indices.push_back(k2);

            // k2 => k1+1 => k2+1
            pObject.indices.push_back(k2);
            pObject.indices.push_back(k1 + 1);
            pObject.indices.push_back(k2 + 1);
        }

        // indices for the base surface
        //NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
        //      please see the previous code snippet
        for(int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if(i < sectorCount - 1)
            {
                pObject.indices.push_back(k);
                pObject.indices.push_back(k + 1);
                pObject.indices.push_back(baseCenterIndex);
            }
            else // last triangle
            {
                pObject.indices.push_back(k);
                pObject.indices.push_back(baseCenterIndex + 1);
                pObject.indices.push_back(baseCenterIndex);
            }
        }

        // indices for the top surface
        for(int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if(i < sectorCount - 1)
            {
                pObject.indices.push_back(k + 1);
                pObject.indices.push_back(k);
                pObject.indices.push_back(topCenterIndex);
            }
            else // last triangle
            {
                pObject.indices.push_back(topCenterIndex + 1);
                pObject.indices.push_back(k);
                pObject.indices.push_back(topCenterIndex);
            }
        }

        return pObject;
    }

    static PrimitiveObject CreateCapsule(){
        PrimitiveObject pObject;

        float x, y, z, xy;   
        float radius = 0.5f;   
        float height = 0.5f;                        // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        int stackCount = 10;
        int sectorCount = 10;
        float sectorStep = 2 * M_PI / sectorCount;
        float stackStep = M_PI / stackCount;
        float sectorAngle, stackAngle;

        for(int i = 0; i <= stackCount / 2 - 1; ++i)
        {
            stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for(int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                
                pObject.vertices.push_back({{x,y,z}, {s,t,t}});
            }
        }

        int indexTop, indexBase;
        indexTop = pObject.vertices.size() - sectorCount - 1; // 1st vertex index at top
        // put side vertices to arrays
        for(int i = 0; i < 2; ++i)
        {
            float h = -height + i * height;           // z value; -h/2 to h/2
            float t = 1.0f - i;                              // vertical tex coord; 1 to 0

            for(int j = 0, k = 0; j <= sectorCount; ++j, k ++)
            {
                float ux = pObject.vertices[pObject.vertices.size() - 1 - k].pos.x;
                float uy = pObject.vertices[pObject.vertices.size() - 1 - k].pos.y;
                float uz = pObject.vertices[pObject.vertices.size() - 1 - k].pos.z;

                pObject.vertices.push_back({{ux * radius * 10, uy * radius * 10, h},{(float)j / sectorCount, t , t}}); 
            }
        }

        indexBase = pObject.vertices.size(); // 1st vertex index at base

        for(int i = stackCount / 2 + 1; i <= stackCount; ++i)
        {
            stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for(int j = 0; j <= sectorCount; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

                // vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                
                pObject.vertices.push_back({{x,y,z - height}, {s,t,t}});
            }
        }

        //---------------------------------------------Index
        int k1, k2;
        for(int i = 0; i < stackCount / 2 - 1; ++i)
        {
            k1 = i * (sectorCount + 1);     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if(i != 0)
                {
                    pObject.indices.push_back(k1 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1);
                }

                // k1+1 => k2 => k2+1
                if(i != (stackCount / 2 - 1))
                {
                    pObject.indices.push_back(k2 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1 + 1);
                }
            }
        }

        // indices for the side surface
        for(int i = 0; i < sectorCount; ++i, ++indexTop, ++indexBase)
        {
            // 2 triangles per sector
            // k1 => k1+1 => k2
            pObject.indices.push_back(indexTop);
            pObject.indices.push_back(indexTop + 1);
            pObject.indices.push_back(indexBase);

            // k2 => k1+1 => k2+1
            pObject.indices.push_back(indexBase);
            pObject.indices.push_back(indexTop + 1);
            pObject.indices.push_back(indexBase + 1);
        }

        int end = (stackCount / 2 * (sectorCount)) + indexBase - 1;

        for(int i = indexBase - sectorCount - 1; i < end ; ++i)
        {
            k1 = i ;     // beginning of current stack
            k2 = k1 + sectorCount + 1;      // beginning of next stack

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding first and last stacks
                // k1 => k2 => k1+1
                if(i != indexBase - 1)
                {
                    pObject.indices.push_back(k1 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1);
                }

                // k1+1 => k2 => k2+1
                if(i != (end - 1))
                {
                    pObject.indices.push_back(k2 + 1);
                    pObject.indices.push_back(k2);
                    pObject.indices.push_back(k1 + 1);
                }
            }
        }

        return pObject;
    }

    inline static PrimitiveObject quad =  {
        {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},        
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
        },
        {
            2, 1, 0, 0, 3, 2
        }
    };

    inline static PrimitiveObject box =  {
        {
            //Back
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},        
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            //Front
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},        
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
            //Left
            {{-1.0f, -1.0f, -1.0f},  {1.0f, 0.0f, 0.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},     
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            //Right
            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},     
            {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
            //Top
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},       
            {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, 
            {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}}, 
            //Bottom
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},       
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, 
            {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
        },
        {
            2, 1, 0, 0, 3, 2,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            14, 13, 12, 12, 15, 14,
            18, 17, 16, 16, 19, 18,
            20, 21, 22, 22, 23, 20
        }
    };
};