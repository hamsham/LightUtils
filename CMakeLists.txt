
# -------------------------------------
# Project Setup
# -------------------------------------
project(LightUtils CXX)
set(OUTPUT_NAME lsutils)



# -------------------------------------
# Source Paths
# -------------------------------------
include_directories("include")

set(LS_UTILS_SOURCES
    "src/utils/Assertions.cpp"
    "src/utils/Copy.cpp"
    "src/utils/DataResource.cpp"
    "src/utils/Pointer.cpp"
    "src/utils/RandomNum.cpp"
    "src/utils/Resource.cpp"
    "src/utils/StringUtils.cpp"
    "src/utils/Tuple.cpp"
)



# -------------------------------------
# Library Setup
# -------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

if (BUILD_SHARED_LIB)
    add_definitions(-DLS_BUILD_SHARED -DLS_SHARED)
    set(LS_LIB_TYPE SHARED)
else()
    set(LS_LIB_TYPE STATIC)
endif()

add_library("${OUTPUT_NAME}" ${LS_LIB_TYPE} "${LS_UTILS_SOURCES}")

# Dependencies
target_link_libraries("${OUTPUT_NAME}" lssetup)



# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS "${OUTPUT_NAME}"
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/lightsky/utils DESTINATION include/lightsky)



# -------------------------------------
# Library Tests
# -------------------------------------
set(LS_BUILD_UTILS_TESTS TRUE CACHE BOOL "Build tests for the LightUtils library.")

if(LS_BUILD_UTILS_TESTS)
    add_subdirectory("tests")
endif()
