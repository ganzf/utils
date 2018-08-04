#
# Utils - Unit Testing
#
cmake_minimum_required(VERSION 3.9)
project(utils)

file(GLOB SOURCES *.hpp)
add_executable(tests src/Basic.cpp ${SOURCES})

if(UNIX)
    target_link_libraries(tests criterion pthread dl)
elseif(WIN32)
    target_link_libraries(tests lib/criterion.lib)
endif()