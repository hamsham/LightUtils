
# -------------------------------------
# Project Setup
# -------------------------------------
project(lsutils_tests CXX)



# -------------------------------------
# Include Paths
# -------------------------------------
include_directories(".")



# -------------------------------------
# Dependency Setup
# -------------------------------------
set(
    LS_UTILS_TEST_DEPS
    lsutils
    lsmath
)



# -------------------------------------
# Building and Linking Targets
# -------------------------------------
add_executable(lsutils_tuple_test "lsutils_tuple_test.cpp")
add_dependencies(lsutils_tuple_test ${LS_UTILS_TEST_DEPS})
