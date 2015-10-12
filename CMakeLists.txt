cmake_minimum_required(VERSION 2.8)

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
    "src/utils/assertions.cpp"
    "src/utils/dataResource.cpp"
    "src/utils/pointer.cpp"
    "src/utils/randomNum.cpp"
    "src/utils/resource.cpp"
    "src/utils/string_utils.cpp"
    "src/utils/tuple.cpp"
)



# -------------------------------------
# Library Setup
# -------------------------------------
add_library("${OUTPUT_NAME}" STATIC "${LS_UTILS_SOURCES}")

# Dependencies
target_link_libraries("${OUTPUT_NAME}" liblssetup)



# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS "${OUTPUT_NAME}" ARCHIVE DESTINATION lib)
install(DIRECTORY include/lightsky/utils DESTINATION include/lightsky)
