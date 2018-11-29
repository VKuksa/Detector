cmake_minimum_required(VERSION 3.12)
project(Detector)

set(CMAKE_CXX_STANDARD 17)
set(VCPKG_LIBRARY_LINKAGE static)

if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x600 -D_UNICODE -DUNICODE)
    add_definitions("/wd4996 /wd4146 /wd4291 /wd4251 /wd4005")
endif ()

find_package(Boost 1.67 COMPONENTS log_setup log program_options REQUIRED)

set(SRC
    src/main.cpp
    )

add_executable(detector ${SRC})
target_include_directories(detector PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(detector PRIVATE ${Boost_LIBRARIES})
