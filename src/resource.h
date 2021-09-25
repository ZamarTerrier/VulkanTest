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
};
