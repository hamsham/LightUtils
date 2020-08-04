
# -------------------------------------
# Project Setup
# -------------------------------------
project(LightUtils CXX)
set(OUTPUT_NAME lsutils)



# -------------------------------------
# 3rd-party libraries
# -------------------------------------
include(external_deps.cmake)
include(enetConfig.cmake)



# -------------------------------------
# Source Paths
# -------------------------------------
include_directories(include)

set(LS_UTILS_SOURCES
    src/Argument.cpp
    src/ArgParser.cpp
    src/Assertions.cpp
    src/Copy.cpp
    src/DataResource.cpp
    src/DynamicLib.cpp
    src/Futex.cpp
    src/RandomNum.cpp
    src/NetClient.cpp
    src/NetConnection.cpp
    src/NetNode.cpp
    src/NetServer.cpp
    src/Resource.cpp
    src/SpinLock.cpp
    src/StringUtils.cpp
    src/Time.cpp
    src/WorkerPool.cpp
    src/WorkerThread.cpp
)

set(LS_UTILS_HEADERS
    include/lightsky/utils/Algorithm.hpp
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
    include/lightsky/utils/generic/AlignedAllocatorImpl.hpp
    include/lightsky/utils/generic/ChunkAllocatorImpl.hpp
    include/lightsky/utils/generic/FutexImpl.hpp
    include/lightsky/utils/generic/GeneralAllocatorImpl.hpp
    include/lightsky/utils/generic/IndexedCacheImpl.hpp
    include/lightsky/utils/generic/LogImpl.h
    include/lightsky/utils/generic/LRUCacheImpl.hpp
    include/lightsky/utils/generic/HashImpl.h
    include/lightsky/utils/generic/SortImpl.hpp
    include/lightsky/utils/generic/SpinLockImpl.hpp
    include/lightsky/utils/generic/WorkerPoolImpl.hpp
    include/lightsky/utils/generic/WorkerThreadImpl.hpp

    include/lightsky/utils/android/LogImpl.h

    include/lightsky/utils/x86/FutexImpl.hpp
)



# -------------------------------------
# Extra Libraries
# -------------------------------------
if (NOT WIN32)
    find_library(LS_UTILS_DL_LIB dl)
    if (LS_UTILS_DL_LIB)
        message("-- Found libdl.so: ${LS_UTILS_DL_LIB}")
    else()
        message(FATAL_ERROR "Could not find libdl.so")
    endif()
else()
    set(LS_UTILS_DL_LIB "")
endif()



# -------------------------------------
# Library Setup
# -------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

if (BUILD_SHARED_LIBS)
    add_definitions(-DLS_BUILD_SHARED -DLS_SHARED)
    set(LS_LIB_TYPE SHARED)
else()
    set(LS_LIB_TYPE STATIC)
endif()

add_library(${OUTPUT_NAME} ${LS_LIB_TYPE} ${LS_UTILS_SOURCES} ${LS_UTILS_HEADERS})

# Dependencies
target_include_directories(${OUTPUT_NAME} PRIVATE ${ENET_INCLUDE_DIR})
target_link_libraries(${OUTPUT_NAME}
    PUBLIC LightSky::Setup
    INTERFACE ${LS_UTILS_DL_LIB} ${ENET_LIBRARIES}
    PUBLIC ${ENET_LIBRARIES}
)



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
# Library Tests
# -------------------------------------
option(LS_BUILD_UTILS_TESTS "Build tests for the LightUtils library." ON)

if(LS_BUILD_UTILS_TESTS)
    add_subdirectory(tests)
endif()
