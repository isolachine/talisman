cmake_minimum_required(VERSION 3.4.3)
project(MediatorAnalysis)
message (STATUS "Searching CMAKE Path in ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}")
find_package(LLVM "10.0.1" REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_CMAKE_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Headers and source for PDG generation.
include_directories("pdg/SVF/include")


include_directories("pdg/include")

# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

# Build type
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\
    -fdiagnostics-color=always -Wno-everything")

include_directories("include")
include_directories("src")

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.h" )

add_library(MediatorAnalysis MODULE
    ${PDG_HEADERS}
    ${HEADERS}
    ${SOURCES}

        test/selinux_cut/selinux_cut.h)



# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(MediatorAnalysis PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
