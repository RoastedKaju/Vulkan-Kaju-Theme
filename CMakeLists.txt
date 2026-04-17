cmake_minimum_required(VERSION 3.24)

project("KajuTheme")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

# ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

target_sources(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Add sub-directories
add_subdirectory("external/glfw")
add_subdirectory("external/vma")
add_subdirectory("external/vk_bootstrap")

find_package(Vulkan REQUIRED)

# Link to target
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan vk-bootstrap GPUOpen::VulkanMemoryAllocator)
