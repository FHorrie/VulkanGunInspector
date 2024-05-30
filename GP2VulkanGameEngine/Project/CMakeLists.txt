# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)


set(SOURCES
 "prim/main.cpp"
 "prim/App.cpp"
 "engine/window.cpp"
 "engine/pipeline.cpp"
 "engine/device.cpp"
 "engine/swapchain.cpp"
 "engine/model.cpp"
 "engine/gameObject.cpp"
 "engine/renderer.cpp"
 "engine/renderSystem.cpp" 
 "engine/FHTime.cpp"
 "camera.cpp"
 "engine/keyboardInput.cpp"
 "engine/buffer.cpp" 
 "engine/descriptors.cpp"
 "engine/texture.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES})
add_dependencies(${PROJECT_NAME} Shaders)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} tinyobjloader)


# Set the directory for resources
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RESOURCES_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

# Custom command to copy the resources folder
add_custom_command(
    TARGET GP2VulkanGameEngine PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${RESOURCES_DIR} ${RESOURCES_BINARY_DIR}
    COMMENT "Copying resources to build directory"
)