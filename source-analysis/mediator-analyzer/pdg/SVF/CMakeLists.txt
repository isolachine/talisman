cmake_minimum_required(VERSION 3.4.3)

project("SVF")

# To support both in- and out-of-source builds,
# we check for the presence of the add_llvm_loadable_module command.
# - if this command is not present, we are building out-of-source
# We need to match the build environment for LLVM:
# In particular, we need C++14 and the -fno-rtti flag
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add -std=gnu++14
set(CMAKE_CXX_EXTENSIONS ON)

set(COMMON_FLAGS "-fPIC -Werror")

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -O0 -fno-rtti")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O0")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -O3 -fno-rtti")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -O3")
endif()

find_package(LLVM "10.0.1" REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

if (LLVM_LINK_LLVM_DYLIB)
	set(llvm_libs LLVM)
else()
	llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support transformutils)
endif()

set(Z3_DIR $ENV{Z3_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
	${Z3_DIR}/include/)

LINK_DIRECTORIES(${Z3_DIR}/bin)
add_subdirectory(lib)
add_subdirectory(tools)

INSTALL(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_BINARY_DIR}/include/ ${Z3_DIR}/include/
	COMPONENT devel
	DESTINATION include/svf
	FILES_MATCHING
	PATTERN "**/*.h"
	)
enable_testing()
#add_subdirectory(Test-Suite)
include(CTest)
