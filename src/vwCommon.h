#pragma once

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <memory>
#include <functional>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

#define VW_CHECK(result)                                                                             \
    if (result != VK_SUCCESS)                                                                        \
    {                                                                                                \
        std::cerr << "VW Check failed at line No: " << __LINE__ << " In file: " << __FILE__ << '\n'; \
        exit(1);                                                                                     \
    }