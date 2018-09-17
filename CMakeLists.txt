
# -------------------------------------
# Project Setup
# -------------------------------------
project(LightUtils CXX)
set(OUTPUT_NAME lsutils)



# -------------------------------------
# Source Paths
# -------------------------------------
include_directories(include)

set(LS_UTILS_SOURCES
    src/Assertions.cpp
    src/Copy.cpp
    src/DataResource.cpp
    src/RandomNum.cpp
    src/Resource.cpp
    src/StringUtils.cpp
    src/WorkerThread.cpp
)

set(LS_UTILS_HEADERS
    include/lightsky/utils/Algorithm.hpp
    include/lightsky/utils/generic/AlgorithmImpl.hpp
    include/lightsky/utils/Assertions.h
    include/lightsky/utils/Bits.h
    include/lightsky/utils/BTree.h
    include/lightsky/utils/ByteSize.h
    include/lightsky/utils/Copy.h
    include/lightsky/utils/DataResource.h
    include/lightsky/utils/Endian.h
    include/lightsky/utils/generic/LogImpl.h
    include/lightsky/utils/android/LogImpl.h
    include/lightsky/utils/Hash.h
    include/lightsky/utils/generic/HashImpl.h
    include/lightsky/utils/Log.h
    include/lightsky/utils/Loops.h
    include/lightsky/utils/Pointer.h
    include/lightsky/utils/RandomNum.h
    include/lightsky/utils/Resource.h
    include/lightsky/utils/Setup.h
    include/lightsky/utils/Sort.hpp
    include/lightsky/utils/generic/SortImpl.hpp
    include/lightsky/utils/StringUtils.h
    include/lightsky/utils/Tuple.h
    include/lightsky/utils/Types.hpp
    include/lightsky/utils/Utils.h
    include/lightsky/utils/WorkerThread.hpp
)



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
target_link_libraries(${OUTPUT_NAME} lssetup)



# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS ${OUTPUT_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/lightsky/utils DESTINATION include/lightsky)



# -------------------------------------
# Library Tests
# -------------------------------------
option(LS_BUILD_UTILS_TESTS "Build tests for the LightUtils library." ON)

if(LS_BUILD_UTILS_TESTS)
    add_subdirectory(tests)
endif()
