cmake_minimum_required(VERSION 3.20)
project(TavernGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. PERCORSI HEADER ---
include_directories(
        lib/glad/include
        lib/glfw-3.3.8.bin.WIN64/include
        lib/glm-master
        include
	src
)

# --- 2. SORGENTI ---
set(SOURCES
        lib/glad/src/glad.c
        src/Main.cpp
        src/stb_image.cpp
        src/Bard.cpp
        src/Customer.cpp
        src/Dirt.cpp
        src/FoodStations.cpp
        src/Player.cpp
        src/TavernObjects.cpp
        src/Wizard.cpp
)

add_executable(TavernGame ${SOURCES})

# --- 3. LOGICA DI SELEZIONE LIBRERIA (Il segreto) ---
find_package(OpenGL REQUIRED)

if(MSVC)
    # Se compili con Visual Studio
    message(STATUS "Rilevato compilatore MSVC (Visual Studio)")
    set(GLFW_LIB "${CMAKE_SOURCE_DIR}/lib/glfw-3.3.8.bin.WIN64/lib-vc2022/glfw3.lib")
else()
    # Se compili con MinGW (CLion default)
    message(STATUS "Rilevato compilatore MinGW/GCC")
    set(GLFW_LIB "${CMAKE_SOURCE_DIR}/lib/glfw-3.3.8.bin.WIN64/lib-mingw-w64/libglfw3.a")
endif()

target_link_libraries(TavernGame OpenGL::GL ${GLFW_LIB})

# --- 4. COPIA ASSET ---
# Questa parte funziona su entrambi per copiare shader e texture
add_custom_command(TARGET TavernGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:TavernGame>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/textures" "$<TARGET_FILE_DIR:TavernGame>/textures"
)