cmake_minimum_required(VERSION 3.10)
project(3D_Platformer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Silence OpenGL deprecation warnings on macOS
if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific setup
if(APPLE)
    # macOS: Use frameworks
    find_library(GLUT_LIBRARY GLUT)
    find_library(OpenGL_LIBRARY OpenGL)

    if(NOT GLUT_LIBRARY)
        message(FATAL_ERROR "GLUT framework not found. Please install freeglut via Homebrew: brew install freeglut")
    endif()

    set(PLATFORM_LIBS ${GLUT_LIBRARY} ${OpenGL_LIBRARY})

    # Add include directories for macOS
    include_directories(/System/Library/Frameworks)

elseif(UNIX)
    # Linux: Use libraries
    find_package(GLUT REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} GLU)
    include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

elseif(WIN32)
    # Windows: You'll need to set up paths to GLUT manually
    find_package(GLUT REQUIRED)
    set(PLATFORM_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
    include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
endif()

# Add executable
add_executable(PXX_YYYY PXX_YYYY.cpp)

# Link libraries
target_link_libraries(PXX_YYYY ${PLATFORM_LIBS})

# Print configuration info
message(STATUS "Build configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  OpenGL Found: ${OPENGL_FOUND}")
if(APPLE)
    message(STATUS "  GLUT Library: ${GLUT_LIBRARY}")
    message(STATUS "  OpenGL Library: ${OpenGL_LIBRARY}")
endif()