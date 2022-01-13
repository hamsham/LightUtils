
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

    if (WIN32)
        if (MINGW)
            set(ENET_LIBRARIES "${ENET_SYS_LIB}" winmm ws2_32)
        else()
            set(ENET_LIBRARIES "${ENET_SYS_LIB}" winmm ws2_32 $<IF:$<CONFIG:Debug,RelWithDebInfo>,msvcrtd,msvcrt> $<IF:$<CONFIG:Debug,RelWithDebInfo>,msvcmrtd,msvcmrt>)
        endif()
    else()
        set(ENET_LIBRARIES "${ENET_SYS_LIB}")
    endif()

else()
    message("-- Building ENet from source")

    # Version Control Tools
    find_package(Git REQUIRED)

    # MSBuild
    if (CMAKE_GENERATOR MATCHES "Xcode" OR CMAKE_VS_MSBUILD_COMMAND)
        if (NOT CMAKE_BUILD_TYPE)
            set(ENET_IDE_BUILD_CONFIG "/$<CONFIG>")
            set(ENET_BUILD_CONFIG "$<CONFIG>")
        else()
            set(ENET_IDE_BUILD_CONFIG "/${CMAKE_BUILD_TYPE}")
            set(ENET_BUILD_CONFIG "${CMAKE_BUILD_TYPE}")
        endif()
	else()
        if (NOT CMAKE_BUILD_TYPE)
            set(ENET_BUILD_CONFIG "RelWithDebInfo")
        else()
            set(ENET_BUILD_CONFIG "${CMAKE_BUILD_TYPE}")
        endif()
    endif()

    if (MSVC)
        set(ENET_C_FLAGS "${CMAKE_C_FLAGS} /p:CharacterSet=Unicode")
        set(ENET_CXX_FLAGS "${CMAKE_CXX_FLAGS} /p:CharacterSet=Unicode")
    else()
        set(ENET_C_FLAGS "${CMAKE_C_FLAGS}")
        set(ENET_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    mark_as_advanced(ENET_CXX_FLAGS)
    mark_as_advanced(ENET_C_FLAGS)

    # Build Setup
    set(ENET_BUILD_FLAGS
        -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS:STRING=${ENET_CXX_FLAGS}
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
        -DCMAKE_C_FLAGS:STRING=${ENET_C_FLAGS}
        -DCMAKE_RC_COMPILER:FILEPATH=${CMAKE_RC_COMPILER}
        -DCMAKE_INSTALL_PREFIX:FILEPATH=${EXTERNAL_PROJECT_PREFIX}
        #-DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
        -DCMAKE_MAKE_PROGRAM:PATH=${CMAKE_MAKE_PROGRAM}
    )
    mark_as_advanced(ENET_BUILD_FLAGS)

    set(ENET_BRANCH "master" CACHE STRING "Git branch or tag for checking out Enet.")
    mark_as_advanced(ENET_BRANCH)


    set(ENET_HEADERS
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/callbacks.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/enet.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/list.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/protocol.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/time.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/types.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/unix.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/utility.h
        ${EXTERNAL_PROJECT_PREFIX}/src/ENet/include/enet/win32.h
    )


    # External build for ENet
    ExternalProject_Add(
        ENet
        PREFIX
            ${EXTERNAL_PROJECT_PREFIX}
        GIT_REPOSITORY
            "https://github.com/lsalzman/enet.git"
        GIT_TAG
            "${ENET_BRANCH}"
        GIT_SHALLOW
            TRUE
        GIT_PROGRESS
            TRUE
        UPDATE_COMMAND
            ${GIT_EXECUTABLE} pull origin ${ENET_BRANCH}
        CMAKE_GENERATOR
            "${CMAKE_GENERATOR}"
        CMAKE_COMMAND
            ${CMAKE_COMMAND}
        CMAKE_CACHE_ARGS
            ${ENET_BUILD_FLAGS}
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_PROJECT_PREFIX}/include/enet &&
            ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/ENet-build${ENET_IDE_BUILD_CONFIG}/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX} ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX} &&
            ${CMAKE_COMMAND} -E copy_if_different ${ENET_HEADERS} ${EXTERNAL_PROJECT_PREFIX}/include/enet
    )

    # Add the imported library target
    add_library(enet STATIC IMPORTED)
    set_target_properties(enet PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${EXTERNAL_PROJECT_PREFIX}/include)
    set_target_properties(enet PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX})
    add_dependencies(enet ENet)
    ExternalProject_Add_Step(ENet lib BYPRODUCTS ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}enet${CMAKE_STATIC_LIBRARY_SUFFIX})

    set(ENET_INCLUDE_DIR "${EXTERNAL_PROJECT_PREFIX}/include")

    if (WIN32)
        if (MINGW)
            set(ENET_LIBRARIES enet winmm ws2_32)
        elseif(MSVC)
            set(ENET_LIBRARIES enet winmm ws2_32 $<IF:$<CONFIG:Debug,RelWithDebInfo>,msvcrtd,msvcrt> $<IF:$<CONFIG:Debug,RelWithDebInfo>,msvcmrtd,msvcmrt>)
        else()
            set(ENET_LIBRARIES enet)
        endif()
    else()
        set(ENET_LIBRARIES enet)
    endif()
endif()

message("-- Found ENet include path: ${ENET_INCLUDE_DIR}")
message("-- Found ENet library: ${ENET_LIBRARIES}")
