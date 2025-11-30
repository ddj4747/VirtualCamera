cmake_minimum_required(VERSION 3.31)
project(VirtualCamera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(${CMAKE_SYSTEM_VERSION})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
    add_library(VirtualCam SHARED
            windows/mf/VirtualCamera.cpp
            windows/mf/VirtualCamera.h
            windows/mf/VirtualCamera.def
    )

    target_compile_definitions(VirtualCam PRIVATE _WINDOWS)

    target_link_libraries(VirtualCam PRIVATE
            Mfplat.lib
            Mf.lib
            Mfuuid.lib
            Uuid.lib
            Ole32.lib
            Kernel32.lib
            Mfsensorgroup.lib
    )
elseif (APPLE)

elseif (UNIX)

endif()