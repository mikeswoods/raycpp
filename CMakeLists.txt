cmake_minimum_required (VERSION 3.0.0)

################################################################################
# To enable OpenMP support in the simulator, uncomment the line below.
# Note: if compiling on OSX (10.8.x Mavericks as I write this), be aware that
# clang/clang++ does not support OpenMP out of the box. You must have g++
# installed and it must be in your current path.
################################################################################

#set (ENABLE_OPENMP 1)

################################################################################

project (raycpp)
find_package(PkgConfig REQUIRED)

# Only use g++ if we're using OpenMP:
if (DEFINED ENABLE_OPENMP)
   MESSAGE("-- Enabled OpenMP")
   set (CMAKE_C_COMPILER gcc)
   set (CMAKE_CXX_COMPILER g++)
   # Let the preprocessor know we're using OpenMP too:
   add_definitions(-DENABLE_OPENMP)
else()
   # Otherwise, explicitly use clang:
   set (CMAKE_C_COMPILER clang)
   set (CMAKE_CXX_COMPILER clang++)
endif ()


# Find and set up core dependency libs:
pkg_search_module(assimp REQUIRED assimp)
pkg_search_module(GLFW REQUIRED glfw3)
find_package (GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS}
                    "include")

set (CORELIBS ${GLFW_LIBRARIES}
              ${GLEW_LIBRARY})

# OSX-specific hacks/fixes
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   #Link IOKit because this is where we get GL stuff for OSX
   set (IOKIT "-framework IOKit")
	set (CORELIBS ${CORELIBS} ${IOKIT})
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Added my own flags: warnings all the way:
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

#Compiler flag magic:
if (UNIX AND NOT APPLE)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -msse2")
elseif (APPLE)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -m64 -msse2")
elseif (WIN32)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

# Remember to add the "-fopenmp" flag when compiling also:
if (DEFINED ENABLE_OPENMP)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif ()

set(CMAKE_EXE_LINKER_FLAGS "-lGL -lassimp")

# Special stuff for Linux/X11:
if (UNIX AND NOT APPLE)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lX11 -pthread -lXrandr -lXcursor -lXinerama")
endif()

# Add all source files. Headers don't need to be listed here since the compiler will find them;
# we just need the actual files being fed directly to the compiler

set (SOURCE_FILES "src/AABB.cpp"
                  "src/AreaLight.cpp"
                  "src/BoundingVolume.cpp"
                  "src/Camera.cpp"
                  "src/Color.cpp"
                  "src/Config.cpp"
                  "src/Cube.cpp"
                  "src/Cylinder.cpp"
                  "src/EnvironmentMap.cpp"
                  "src/Geometry.cpp"
                  "src/GLGeometry.cpp"
                  "src/GLUtils.cpp"
                  "src/GLWorldState.cpp"
                  "src/GraphBuilder.cpp"
                  "src/Graph.cpp"
                  "src/Image.cpp"
                  "src/Intersection.cpp"
                  "src/KDTree.cpp"
                  "src/Light.cpp"
                  "src/main.cpp"
                  "src/Material.cpp"
                  "src/Mesh.cpp"
                  "src/ModelImport.cpp"
                  "src/NormalMap.cpp"
                  "src/PointLight.cpp"
                  "src/Ray.cpp"
                  "src/Raytrace.cpp"
                  "src/Sampling.cpp"
                  "src/SceneContext.cpp"
                  "src/Sphere.cpp"
                  "src/SurfaceMap.cpp"
                  "src/Tri.cpp"
                  "src/Utils.cpp")

add_executable(raycpp ${SOURCE_FILES})

target_link_libraries (raycpp ${CORELIBS})
