
# #############################################################################
# ENet Library
# #############################################################################
find_path(ENET_SYS_INCLUDE_DIR enet/enet.h)

find_library(ENET_SYS_LIB
    NAMES
        ${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${CMAKE_SHARED_LIBRARY_PREFIX}enet${CMAKE_SHARED_LIBRARY_SUFFIX}
    HINTS
        ${ENET_SYS_INCLUDE_DIR}/../lib
        ${ENET_SYS_INCLUDE_DIR}/../lib64
)

option(BUILD_ENET "Force ENet to build from source." OFF)

if (NOT BUILD_ENET AND NOT ENET_SYS_INCLUDE_DIR STREQUAL ENET_SYS_INCLUDE_DIR-NOTFOUND AND NOT ENET_SYS_LIB STREQUAL ENET_SYS_LIB-NOTFOUND)
    set(ENET_INCLUDE_DIR "${ENET_SYS_INCLUDE_DIR}")
    set(ENET_LIBRARIES "${ENET_SYS_LIB}")

else()
    message("-- Building ENet from source")

    # MSBuild
    if (NOT CMAKE_BUILD_TYPE AND CMAKE_VS_MSBUILD_COMMAND)
        if (NOT CMAKE_BUILD_TYPE)
            set(ENET_WINDOWS_BUILD_CONFIG "/Debug")
        else()
            set(ENET_WINDOWS_BUILD_CONFIG "/${CMAKE_BUILD_TYPE}")
        endif()
    endif()

    # Version Control Tools
    find_package(Git REQUIRED)

    # Build Setup
    set(ENET_BUILD_FLAGS
        -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
        -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
        -DCMAKE_RC_COMPILER:FILEPATH=${CMAKE_RC_COMPILER}
        -DCMAKE_INSTALL_PREFIX:FILEPATH=${EXTERNAL_PROJECT_PREFIX}
        -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME})
    mark_as_advanced(ENET_BUILD_FLAGS)

    set(ENET_BRANCH "master" CACHE STRING "Git branch or tag for checking out Enet.")
    mark_as_advanced(ENET_BRANCH)



    # External build for ENet
    ExternalProject_Add(
        ENet
        PREFIX
            ${EXTERNAL_PROJECT_PREFIX}
        GIT_REPOSITORY
            "https://github.com/lsalzman/enet.git"
        GIT_SHALLOW
            TRUE
        GIT_PROGRESS
            TRUE
        GIT_TAG
            "${ENET_BRANCH}"
        UPDATE_COMMAND
            ${GIT_EXECUTABLE} pull origin ${ENET_BRANCH}
        CMAKE_COMMAND
            ${CMAKE_COMMAND}
        CMAKE_ARGS
            ${ENET_BUILD_FLAGS}
        CMAKE_CACHE_ARGS
            ${ENET_BUILD_FLAGS}
        BUILD_COMMAND
            ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_PREFIX}/src/ENet-build ${CMAKE_COMMAND} --build . --config ${CMAKE_CFG_INTDIR}
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/ENet-build${ENET_WINDOWS_BUILD_CONFIG}/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX} ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX} &&
            ${CMAKE_COMMAND} -E copy_directory ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet ${EXTERNAL_PROJECT_PREFIX}/include/enet
    )

    # Add the imported library target
    add_library(enet STATIC IMPORTED)
    set_target_properties(enet PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${EXTERNAL_PROJECT_PREFIX}/include)
    set_target_properties(enet PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX})
    add_dependencies(enet ENet)

    set(ENET_INCLUDE_DIR "${EXTERNAL_PROJECT_PREFIX}/include")
    set(ENET_LIBRARIES enet)
endif()

message("-- Found ENet include path: ${ENET_INCLUDE_DIR}")
message("-- Found ENet library: ${ENET_LIBRARIES}")
