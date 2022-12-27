
# -------------------------------------
# Project Setup
# -------------------------------------
project(ls_utils_tests CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)



# -------------------------------------
# Include Paths
# -------------------------------------
include_directories(".")



# -------------------------------------
# Building and Linking Targets
# -------------------------------------
find_package(Threads REQUIRED)

function(LS_UTILS_ADD_TARGET target sources)
    add_executable(${target} ${sources} ${ARGN})
	target_link_libraries(${target} LightSky::Utils LightSky::Setup Threads::Threads)
	ls_configure_target(${target})

	# Precompiled Headers
	if (LIGHTSKY_ENABLE_PCH)
		target_precompile_headers(${target} REUSE_FROM lsutils)
	endif()
endfunction(LS_UTILS_ADD_TARGET)

LS_UTILS_ADD_TARGET(lsutils_alloc_chunk_test   lsutils_alloc_chunk_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_alloc_general_test lsutils_alloc_general_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_argparse_test      lsutils_argparse_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_cache_test         lsutils_cache_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_dylib_test         lsutils_dylib_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_memcpy_test        lsutils_memcpy_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_memset_test        lsutils_memset_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_net_client_test    lsutils_net_test.hpp lsutils_net_client_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_net_server_test    lsutils_net_test.hpp lsutils_net_server_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_sort_test          lsutils_sort_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_to_str_test        lsutils_to_str_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_tuple_test         lsutils_tuple_test.cpp)
LS_UTILS_ADD_TARGET(lsutils_worker_test        lsutils_worker_test.cpp)

add_library(lsmalloc
	SHARED
		lsutils_libmalloc_test.cpp
		${LightUtils_SOURCE_DIR}/src/Allocator.cpp
		${LightUtils_SOURCE_DIR}/src/Assertions.cpp
		${LightUtils_SOURCE_DIR}/src/GeneralAllocator.cpp
		${LightUtils_SOURCE_DIR}/src/MemorySource.cpp
		${LightUtils_SOURCE_DIR}/src/SpinLock.cpp
)

target_compile_definitions(lsmalloc PRIVATE -DLS_BUILD_SHARED=1)
ls_configure_target(lsmalloc)
target_link_libraries(lsmalloc LightSky::Utils LightSky::Setup Threads::Threads)

if (ENABLE_VALGRIND_TRACKING)
	target_compile_definitions(lsmalloc PUBLIC -DLS_VALGRIND_TRACKING)
endif ()

add_dependencies(lsutils_alloc_general_test lsmalloc)
