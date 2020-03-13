cmake_minimum_required(VERSION 3.9)
project(jana2 VERSION 2.0.1)

# C++ standard to use (controlled with -DCPP_STANDARD flag)
if(NOT DEFINED CMAKE_CXX_STANDARD)
    SET(CMAKE_CXX_STANDARD 14)
    message(STATUS "jana: CMAKE_CXX_STANDARD set default to ${CMAKE_CXX_STANDARD}")
endif()
message(STATUS "jana: CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Enable -fPIC for all targets

# Expose custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Require the user to specify CMAKE_INSTALL_PREFIX directly. DO NOT AUTOMATICALLY INSTALL to /usr/local!
# Remember that we ultimately want CMAKE_INSTALL_PREFIX=$ENV{JANA_HOME}
if(NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")
else()
    message(FATAL_ERROR "Please provide CMAKE_INSTALL_PREFIX or JANA_HOME (as a CMake variable, not an envvar)")
endif()

include_directories(src/libraries)   # So that everyone can find the JANA header files

add_subdirectory(src/libraries/JANA)
add_subdirectory(src/examples)

add_subdirectory(src/plugins/JTest)
#add_subdirectory(src/plugins/streamDet)
add_subdirectory(src/plugins/janapy)
add_subdirectory(src/plugins/janarate)
#add_subdirectory(src/plugins/janacontrol)

add_subdirectory(src/programs/jana)
add_subdirectory(src/programs/tests)

install(PROGRAMS scripts/jana-generate.py scripts/jana-upgrade.py DESTINATION bin)

include(${CMAKE_SOURCE_DIR}/cmake/MakeConfig.cmake)
