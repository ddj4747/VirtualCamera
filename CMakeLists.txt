cmake_minimum_required(VERSION 3.31)
project(VirtualCamera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (WIN32)
    set(SOURCES
            windows/mf/DllMain.cpp
            windows/mf/Global.h
            windows/mf/VirtualCamera.h
            windows/mf/VCamContext.cpp
            windows/mf/VCamContext.h
    )

    add_library(VirtualCam SHARED
            ${SOURCES}
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

    target_include_directories(VirtualCam PUBLIC windows/mf/VirtualCamera.h)
elseif (APPLE)

elseif (UNIX)

endif()