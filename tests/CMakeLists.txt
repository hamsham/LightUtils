
# -------------------------------------
# Project Setup
# -------------------------------------
project(ls_utils_tests CXX)



# -------------------------------------
# Include Paths
# -------------------------------------
include_directories(".")



# -------------------------------------
# Building and Linking Targets
# -------------------------------------
function(LS_UTILS_ADD_TARGET target sources)
    add_executable(${target} ${sources} ${ARGN})
	if (NOT MSVC)
		target_link_libraries(${target} LightSky::Utils LightSky::Setup pthread)
	else()
		target_link_libraries(${target} LightSky::Utils LightSky::Setup)
	endif()
endfunction(LS_UTILS_ADD_TARGET)

LS_UTILS_ADD_TARGET(lsutils_argparse_test  lsutils_argparse_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_dylib_test     lsutils_dylib_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_memcpy_test    lsutils_memcpy_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_memset_test    lsutils_memset_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_net_client_test lsutils_net_test.hpp lsutils_net_client_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_net_server_test lsutils_net_test.hpp lsutils_net_server_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_sort_test      lsutils_sort_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_to_str_test    lsutils_to_str_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_tuple_test     lsutils_tuple_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_worker_test    lsutils_worker_test.cpp)

