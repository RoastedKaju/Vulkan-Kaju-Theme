cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(RenderingWithVulkan)

add_subdirectory("external/glfw")
add_subdirectory("external/glm")
add_subdirectory("external/volk")

find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_executable(${PROJECT_NAME})
file(GLOB_RECURSE SRC_FILES "source/*.cpp" "source/*.hpp")
target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "source")

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    Vulkan::shaderc_combined
    glfw
    glm
    volk
)

target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)