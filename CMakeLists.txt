
# -------------------------------------
# Project Setup
# -------------------------------------
project(LightUtils CXX)
set(OUTPUT_NAME lsutils)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)



# -------------------------------------
# 3rd-party libraries
# -------------------------------------
include(FindPackageHandleStandardArgs)

include(enetConfig.cmake)

find_path(VALGRIND_INCLUDE_DIR valgrind/valgrind.h)
find_path(MEMCHECK_INCLUDE_DIR valgrind/memcheck.h)
find_package_handle_standard_args(Valgrind DEFAULT_MSG VALGRIND_INCLUDE_DIR MEMCHECK_INCLUDE_DIR)

if (Valgrind_FOUND)
    option(ENABLE_VALGRIND_TRACKING "Enable memory tracking in custom allocators with Valgrind." OFF)

    if (ENABLE_VALGRIND_TRACKING)
        message("-- Enabled custom allocator tracking with Valgrind.")
    endif ()
endif ()



# -------------------------------------
# Source Paths
# -------------------------------------
set(LS_UTILS_SOURCES
    src/Allocator.cpp
    src/Argument.cpp
    src/ArgParser.cpp
    src/Assertions.cpp
    src/Copy.cpp
    src/DataResource.cpp
    src/DynamicLib.cpp
    src/Futex.cpp
    src/GeneralAllocator.cpp
    src/MemorySource.cpp
    src/NetClient.cpp
    src/NetConnection.cpp
    src/NetNode.cpp
    src/NetServer.cpp
    src/RandomNum.cpp
    src/Resource.cpp
    src/SpinLock.cpp
    src/StringUtils.cpp
    src/Time.cpp
    src/WorkerPool.cpp
    src/WorkerThread.cpp
)

set(LS_UTILS_HEADERS
    include/lightsky/utils/Algorithm.hpp
    include/lightsky/utils/Allocator.hpp
    include/lightsky/utils/Argument.hpp
    include/lightsky/utils/ArgParser.hpp
    include/lightsky/utils/Assertions.h
    include/lightsky/utils/Bits.h
    include/lightsky/utils/BTree.h
    include/lightsky/utils/ByteSize.h
    include/lightsky/utils/ChunkAllocator.hpp
    include/lightsky/utils/Copy.h
    include/lightsky/utils/DataResource.h
    include/lightsky/utils/DynamicLib.hpp
    include/lightsky/utils/Endian.h
    include/lightsky/utils/Futex.hpp
    include/lightsky/utils/GeneralAllocator.hpp
    include/lightsky/utils/Hash.h
    include/lightsky/utils/IndexedCache.hpp
    include/lightsky/utils/Log.h
    include/lightsky/utils/Loops.h
    include/lightsky/utils/LRUCache.hpp
    include/lightsky/utils/LRU8WayCache.hpp
    include/lightsky/utils/MemorySource.hpp
    include/lightsky/utils/NetClient.hpp
    include/lightsky/utils/NetConnection.hpp
    include/lightsky/utils/NetEvent.hpp
    include/lightsky/utils/NetNode.hpp
    include/lightsky/utils/NetServer.hpp
    include/lightsky/utils/Pointer.h
    include/lightsky/utils/RandomNum.h
    include/lightsky/utils/Resource.h
    include/lightsky/utils/Setup.h
    include/lightsky/utils/Sort.hpp
    include/lightsky/utils/SpinLock.hpp
    include/lightsky/utils/StringUtils.h
    include/lightsky/utils/Time.hpp
    include/lightsky/utils/Tuple.h
    include/lightsky/utils/Utils.h
    include/lightsky/utils/WorkerPool.hpp
    include/lightsky/utils/WorkerThread.hpp

    include/lightsky/utils/generic/AlgorithmImpl.hpp
    include/lightsky/utils/generic/AllocatorImpl.hpp
    include/lightsky/utils/generic/BTreeImpl.hpp
    include/lightsky/utils/generic/ChunkAllocatorImpl.hpp
    include/lightsky/utils/generic/FutexImpl.hpp
    include/lightsky/utils/generic/GeneralAllocatorImpl.hpp
    include/lightsky/utils/generic/IndexedCacheImpl.hpp
    include/lightsky/utils/generic/LRUCacheImpl.hpp
    include/lightsky/utils/generic/LRU8WayCacheImpl.hpp
    include/lightsky/utils/generic/HashImpl.h
    include/lightsky/utils/generic/SortImpl.hpp
    include/lightsky/utils/generic/SpinLockImpl.hpp
    include/lightsky/utils/generic/WorkerPoolImpl.hpp
    include/lightsky/utils/generic/WorkerThreadImpl.hpp
)

set(LS_UTILS_PLATFORM_HEADERS
    include/lightsky/utils/generic/AlignedAllocatorImpl.hpp  # Causes issues with PCH
    include/lightsky/utils/generic/LogImpl.h
    include/lightsky/utils/android/LogImpl.h

)



# -------------------------------------
# Extra Libraries
# -------------------------------------
if (NOT WIN32)
    find_library(LS_UTILS_DL_LIB NAMES libdl.so dl)
    if (LS_UTILS_DL_LIB)
        message("-- Found libdl.so: ${LS_UTILS_DL_LIB}")
    else()
        message(FATAL_ERROR "Could not find libdl.so")
    endif()
else()
    set(LS_UTILS_DL_LIB "")
endif()

find_package(Threads REQUIRED)



# -------------------------------------
# Library Setup
# -------------------------------------
add_library(${OUTPUT_NAME} ${LS_UTILS_SOURCES} ${LS_UTILS_HEADERS} ${LS_UTILS_PLATFORM_HEADERS})

ls_configure_target(${OUTPUT_NAME})
target_include_directories(${OUTPUT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_include_directories(${OUTPUT_NAME} PRIVATE ${ENET_INCLUDE_DIR})
target_link_libraries(${OUTPUT_NAME}
    PUBLIC
        LightSky::Setup
        ${LS_UTILS_DL_LIB}
        ${ENET_LIBRARIES}
        Threads::Threads
)

if (ENABLE_VALGRIND_TRACKING)
    target_compile_definitions(${OUTPUT_NAME} PUBLIC -DLS_VALGRIND_TRACKING)
endif ()




# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS ${OUTPUT_NAME}
    EXPORT LightUtils
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/lightsky/utils DESTINATION include/lightsky)

install(EXPORT LightUtils
    FILE LightUtils.cmake
    NAMESPACE LightSky::
    DESTINATION lib/cmake/LightSky
)

export(EXPORT LightUtils NAMESPACE LightSky::)
add_library(LightSky::Utils ALIAS lsutils)



# -------------------------------------
# Precompiled Headers
# -------------------------------------
if (LIGHTSKY_ENABLE_PCH)
    target_precompile_headers(
        lsutils
        PRIVATE
            <condition_variable>
            <limits>
            <map>
            <memory>
            <mutex>
            <set>
            <string>
            <thread>
            <unordered_map>
            <unordered_set>
            <utility>
            <vector>
            ${LS_UTILS_HEADERS}
    )
endif()



# -------------------------------------
# Library Tests
# -------------------------------------
option(LS_UTILS_BUILD_TESTS "Build tests for the LightUtils library." ON)

if(LS_UTILS_BUILD_TESTS)
    add_subdirectory(tests)
endif()
