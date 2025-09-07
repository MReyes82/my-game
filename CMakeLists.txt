cmake_minimum_required(VERSION 3.20)

project(CimaEngine3v LANGUAGES CXX)

set(BIN "${CMAKE_SOURCE_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN}/${CMAKE_BUILD_TYPE}")

if (UNIX)
    set(CMAKE_PREFIX_PATH "/home/linuxbrew/.linuxbrew" CACHE PATH "homebrew prefix")
    set(SFML_DIR "/home/linuxbrew/.linuxbrew/opt/sfml@3/lib/cmake/SFML")
endif()

add_compile_definitions(ASSETS="${CMAKE_SOURCE_DIR}/assets")
#bandera para DEBUG O RELEASE
add_compile_definitions("DEBUG=$<CONFIG:Debug>")

#Descargar las dependencias (IMGUI y SFML)
message("Descargando SFML")
include(FetchContent)
if(WIN32)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY http://github.com/SFML/SFML.git
    GIT_TAG 3.0.x)
FetchContent_MakeAvailable(SFML)
else()
    find_package(SFML 3 COMPONENTS System Window Graphics Audio REQUIRED)
endif()
message("Descargando IMGUI")

FetchContent_Declare(
    imgui
    GIT_REPOSITORY http://github.com/ocornut/imgui.git
    GIT_TAG v1.91.1-docking)
FetchContent_MakeAvailable(imgui)

message("Descargando ImguiSFML")

FetchContent_Declare(
    imgui-sfml
    GIT_REPOSITORY http://github.com/SFML/imgui-sfml.git
    GIT_TAG v3.0)

set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

FetchContent_MakeAvailable(imgui-sfml)
#incluir los headers
include_directories(${SFML_INCLUDE_DIRS})
include_directories(${IMGUI_DIR})
#include_directories(${S_INCLUDE_DIRS})

#incluir los folders
add_subdirectory(src)


#ligar las librerias
target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    SFML::System SFML::Window SFML::Graphics SFML::Audio
    ImGui-SFML::ImGui-SFML
)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_options(-ggdb -Wall -Wextra)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
else()
    add_compile_options(-O2)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

#asignar el estandar
target_compile_features(CimaEngine3v PUBLIC cxx_std_17)
