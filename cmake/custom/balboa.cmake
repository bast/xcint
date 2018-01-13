include(ExternalProject)

ExternalProject_Add(
    balboa
    PREFIX "${PROJECT_BINARY_DIR}/balboa"
    GIT_REPOSITORY https://github.com/bast/balboa.git
    GIT_TAG b7fc688e890ad728e07d505802eedbaf54a1ad9e
    INSTALL_COMMAND true  # currently no install command
    CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    )

link_directories(${PROJECT_BINARY_DIR}/balboa/src/balboa-build/lib)

# workaround:
# different dynamic lib suffix on mac
if(APPLE)
    set(_dyn_lib_suffix dylib)
else()
    set(_dyn_lib_suffix so)
endif()

set(BALBOA_LIBS
    libbalboa.${_dyn_lib_suffix}
    )

include_directories(${PROJECT_BINARY_DIR}/balboa/src/balboa/balboa)
include_directories(${PROJECT_BINARY_DIR}/balboa/src/balboa-build/include)
