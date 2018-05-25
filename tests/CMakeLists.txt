
# -------------------------------------
# Project Setup
# -------------------------------------
project(ls_utils_tests CXX)



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
function(LS_UTILS_ADD_TARGET target sources)
    add_executable(${target} ${sources} ${ARGN})
    target_link_libraries(${target} lsutils pthread)
endfunction(LS_UTILS_ADD_TARGET)

LS_UTILS_ADD_TARGET(ls_utils_tuple_test lsutils_tuple_test.cpp)
LS_UTILS_ADD_TARGET(ls_utils_worker_test lsutils_worker_test.cpp)
