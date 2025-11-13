cmake_minimum_required(VERSION 3.16)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")
project(boostio)

if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)
    endif()
endif()

set(EXECUTABLE_NAME ${PROJECT_NAME})
if (ANDROID)
    set(EXECUTABLE_NAME main)
    add_library(${EXECUTABLE_NAME} SHARED)
else()
    add_executable(${EXECUTABLE_NAME})
endif()

target_sources(${EXECUTABLE_NAME}
PRIVATE
    src/main.c
    src/app/app_state.c
    src/app/app_controller.c
    src/app/lua_command_registry.c
    src/app/lua_service.c
    src/app/ui_components/button.c
    src/core/platform/platform.c
    src/core/lua/lua_runtime.c
    src/core/lua/lua_api.c
    src/core/graphics/window.c
    src/core/graphics/graphics.c
    src/core/graphics/msdf_atlas.c
    src/core/graphics/shader.c
    src/core/graphics/primitive_buffer.c
    src/core/input/input_mapper.c
    src/core/input/input_handler.c
    src/core/audio/audio.c
    src/core/audio/synth.c
    src/core/audio/sequencer.c
    src/core/audio/song_loader.c
    external/glad_generated/src/gl.c
    external/cJSON/cJSON.c
)

target_compile_features(${EXECUTABLE_NAME} PUBLIC c_std_11)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED)
find_package(Lua REQUIRED)

add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
add_subdirectory(external/freetype EXCLUDE_FROM_ALL)
add_subdirectory(msdf-atlas-gen)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC
    SDL3::SDL3
    OpenGL::GL
    PNG::PNG
    freetype
    msdf-atlas-gen
    ${LUA_LIBRARIES}
    m
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/msdf-atlas-gen
    ${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/msdf-atlas-gen
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad_generated/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON
    ${LUA_INCLUDE_DIR}
)

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
    COMMENT "Copying assets to build directory"
)

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/lua
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lua
    COMMENT "Copying Lua files to build directory"
)

if(IOS)
    set(RESOURCE_FILES "src/logo.png")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.example.sdl3-app"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.example.sdl3-app"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    install(TARGETS ${EXECUTABLE_NAME}
        BUNDLE DESTINATION ./install COMPONENT Runtime
        RUNTIME DESTINATION ./install/bin COMPONENT Runtime
    )
    set(DEP_DIR "${CMAKE_BINARY_DIR}")
    INSTALL(CODE
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        "
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()
