cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SimpleEngine LANGUAGES CXX)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(imgui CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_program(SLANGC_EXECUTABLE NAMES slangc REQUIRED)

file(GLOB_RECURSE CORE_SOURCES 
    CONFIGURE_DEPENDS 
    "src/**/*.cpp"
    "src/**/*.hpp"
)
list(FILTER CORE_SOURCES EXCLUDE REGEX "src/main.cpp$")

add_library(EngineCore STATIC ${CORE_SOURCES})
target_include_directories(EngineCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(EngineCore PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(EngineCore PUBLIC
    Vulkan::Vulkan
    glfw
    glm::glm
    tinyobjloader::tinyobjloader
    imgui::imgui
)
target_compile_definitions(EngineCore PUBLIC
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE EngineCore)

add_executable(Tests 
    tests/test_main.cpp
    tests/test_entity_components.cpp
)
target_link_libraries(Tests PRIVATE EngineCore Catch2::Catch2WithMain)

set(SHADERS_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})
file(GLOB_RECURSE SLANG_SOURCES "${SHADERS_SRC_DIR}/*.slang")
set(SPV_OUTPUTS "")

foreach(SHADER_PATH ${SLANG_SOURCES})
  get_filename_component(SHADER_FULL_NAME ${SHADER_PATH} NAME)

  string(REGEX REPLACE "\\.slang$" ".spv" SHADER_OUTPUT_NAME ${SHADER_FULL_NAME})
  set(OUTPUT_SPV "${SHADERS_OUT_DIR}/${SHADER_OUTPUT_NAME}")

  if(SHADER_FULL_NAME MATCHES "\\.vert\\.slang$")
    set(ENTRY_POINT_FLAG "-entry;vertMain")
  elseif(SHADER_FULL_NAME MATCHES "\\.frag\\.slang$")
    set(ENTRY_POINT_FLAG "-entry;fragMain")
  else()
    set(ENTRY_POINT_FLAG "-entry;main")
  endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SLANG_DEBUG_FLAGS "-g2;-O0")
  else()
    set(SLANG_DEBUG_FLAGS "-O3")
  endif()

  add_custom_command(
    OUTPUT ${OUTPUT_SPV}
    COMMAND ${SLANGC_EXECUTABLE} ${SHADER_PATH} -target spirv -profile spirv_1_4 -emit-spirv-directly -matrix-layout-column-major -fvk-use-entrypoint-name ${ENTRY_POINT_FLAG} ${SLANG_DEBUG_FLAGS} -o ${OUTPUT_SPV}
    DEPENDS ${SHADER_PATH}
    COMMENT "Compiling shader: ${SHADER_FULL_NAME}"
    VERBATIM
  )
  list(APPEND SPV_OUTPUTS ${OUTPUT_SPV})
endforeach()

add_custom_target(CompileShaders ALL DEPENDS ${SPV_OUTPUTS})

set(RES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(RES_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/resources")
file(MAKE_DIRECTORY ${RES_OUT_DIR})
file(GLOB_RECURSE RESOURCE_FILES 
    CONFIGURE_DEPENDS
    "${RES_SRC_DIR}/*"
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

add_dependencies(EngineCore CopyResources)
add_dependencies(EngineCore CompileShaders)
