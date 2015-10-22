
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
add_library("${OUTPUT_NAME}" STATIC "${LS_UTILS_SOURCES}")

# Dependencies
target_link_libraries("${OUTPUT_NAME}" lssetup)



# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS "${OUTPUT_NAME}" ARCHIVE DESTINATION lib)
install(DIRECTORY include/lightsky/utils DESTINATION include/lightsky)



# -------------------------------------
# Library Tests
# -------------------------------------
set(LS_BUILD_UTILS_TESTS TRUE CACHE BOOL "Build tests for the LightUtils library.")

if(LS_BUILD_UTILS_TESTS)
    add_subdirectory("tests")
endif()
