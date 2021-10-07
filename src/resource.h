#pragma once

#include "stdinclude.h"

#define KEYS 349

class Resource
{
public:
    inline static VkCommandPool commandPool;
    inline static bool pressed[KEYS];
    inline static uint32_t currentImage;
    inline static VkExtent2D swapChainExtent;
    inline static VkFormat swapChainImageFormat;
    inline static size_t countFrames;
    inline static glm::vec3 sunDir;
    inline static bool showCursor;

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }
};
