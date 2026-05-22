cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SimpleEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS 
    "src/**/*.cpp"
    "src/*.cpp"
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})
file(GLOB_RECURSE SLANG_SOURCES "${SHADERS_SRC_DIR}/*.slang")

set(SPV_OUTPUTS "")

foreach(SHADER_PATH ${SLANG_SOURCES})
  get_filename_component(SHADER_NAME ${SHADER_PATH} NAME_WE)
  set(OUTPUT_SPV "${SHADERS_OUT_DIR}/${SHADER_NAME}.spv")
  add_custom_command(
    OUTPUT ${OUTPUT_SPV}
    COMMAND ${SLANGC_EXECUTABLE} ${SHADER_PATH} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o ${OUTPUT_SPV}
    DEPENDS ${SHADER_PATH}
    COMMENT "Compiling shader: ${SHADER_NAME}.slang"
    VERBATIM
  )

  list(APPEND SPV_OUTPUTS ${OUTPUT_SPV})
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPV_OUTPUTS})
add_dependencies(${PROJECT_NAME} CompileShaders)

set(RES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RES_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")

file(MAKE_DIRECTORY ${RES_OUT_DIR})

file(GLOB_RECURSE RESOURCE_FILES 
    CONFIGURE_DEPENDS
    "${RES_SRC_DIR}/*.png"
    "${RES_SRC_DIR}/*.jpg"
    "${RES_SRC_DIR}/*.obj"
    "${RES_SRC_DIR}/*.mtl"
)

set(RES_OUTPUTS "")

foreach(RES_PATH ${RESOURCE_FILES})
    file(RELATIVE_PATH REL_RES ${RES_SRC_DIR} ${RES_PATH})
    set(OUTPUT_RES "${RES_OUT_DIR}/${REL_RES}")

    get_filename_component(OUT_SUBDIR ${OUTPUT_RES} DIRECTORY)
    
    add_custom_command(
        OUTPUT ${OUTPUT_RES}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_SUBDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RES_PATH} ${OUTPUT_RES}
        DEPENDS ${RES_PATH}
        COMMENT "Copying resource: ${REL_RES}"
        VERBATIM
    )

    list(APPEND RES_OUTPUTS ${OUTPUT_RES})
endforeach()

add_custom_target(CopyResources ALL DEPENDS ${RES_OUTPUTS})
add_dependencies(${PROJECT_NAME} CopyResources)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
    imgui::imgui
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)
