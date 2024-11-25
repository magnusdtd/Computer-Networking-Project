cmake_minimum_required(VERSION 3.10)

project(RemoteDesktopWithGmailAPI)

# Use vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify target architecture
if(NOT DEFINED CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64)
endif()

# Set the prefix path for vcpkg
list(APPEND CMAKE_PREFIX_PATH "D:/vcpkg/installed/x64-windows")

# Find the required packages
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OpenCV REQUIRED)

# Server
# Add executable
add_executable(
    server 
    ./Server/server.cpp 
    ./Server/ServerSocket.hpp 
    ./Server/ServerSocket.cpp 
    ./WindowAPI/Keylogger.hpp
    ./WindowAPI/Keylogger.cpp
    ./WindowAPI/KeyboardDisabler.hpp
    ./WindowAPI/KeyboardDisabler.cpp
    ./WindowAPI/VideoRecorder.hpp
    ./WindowAPI/VideoRecorder.cpp
    ./WindowAPI/FileOperations.hpp
    ./WindowAPI/FileOperations.cpp
    ./WindowAPI/ProcessOperations.hpp
    ./WindowAPI/ProcessOperations.cpp
    ./WindowAPI/SystemOperations.hpp
    ./WindowAPI/SystemOperations.cpp
    ./WindowAPI/MyUtility.hpp
    ./WindowAPI/MyUtility.cpp
)

# Link necessary libraries
target_link_libraries(server PRIVATE ws2_32 ${OpenCV_LIBS})

# Include OpenCV directories
target_include_directories(server PRIVATE ${OpenCV_INCLUDE_DIRS})

# Client
add_executable(
    client 
    ./Client/client.cpp 
    ./Client/ClientSocket.hpp 
    ./Client/ClientSocket.cpp
    ./GmailAPI/GmailAPI.hpp
    ./GmailAPI/GmailAPI.cpp
)

target_link_libraries(client PRIVATE ws2_32 nlohmann_json::nlohmann_json CURL::libcurl OpenSSL::SSL)

# Custom command to copy directory after build
add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/Code/Socket/Final/build/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "D:/vcpkg/installed/x64-windows/debug/bin"
    "D:/Code/Socket/Final/build/Debug")

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/Code/Socket/Final/build/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "D:/vcpkg/installed/x64-windows/debug/bin"
    "D:/Code/Socket/Final/build/Debug")

#===============================================================================
# Google Test
find_package(GTest REQUIRED)

add_executable(
    tests 
    ./Tests/test_main.cpp 
)

target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main)

add_custom_command(TARGET tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/Code/Socket/Final/build/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "D:/vcpkg/installed/x64-windows/debug/bin"
    "D:/Code/Socket/Final/build/Debug")