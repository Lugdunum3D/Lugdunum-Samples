cmake_minimum_required(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules")

# use sample' macros
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Macros.cmake)

# determine the build type
lug_set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")

if(ANDROID)
    populate_android_infos()
endif()

# set the path of thirdparty
lug_set_option(LUG_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty" STRING "Choose the path for the thirdparty directory")

# project name
set(SAMPLE_NAME sample_03)
project(${SAMPLE_NAME})

# use config
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Config.cmake)

set(SRC
    src/Application.cpp
    src/main.cpp
)
source_group("src" FILES ${SRC})

set(INC
    include/Application.hpp
)
source_group("inc" FILES ${INC})

set(SHADERS
    gui.frag
    gui.vert
)

set(LUG_RESOURCES
    shaders/forward/shader.frag
    shaders/forward/shader.vert
)

set(OTHER_RESOURCES

)

include_directories(include)

lug_add_sample(${SAMPLE_NAME}
               SOURCES ${SRC} ${INC}
               DEPENDS core graphics system window math
               SHADERS ${SHADERS}
               LUG_RESOURCES ${LUG_RESOURCES}
               OTHER_RESOURCES ${OTHER_RESOURCES}
)
