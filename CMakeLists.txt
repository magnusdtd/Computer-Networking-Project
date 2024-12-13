cmake_minimum_required(VERSION 3.10)

project(RemoteDesktopWithGmailAPI LANGUAGES CXX)

# Use vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the prefix path for vcpkg
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/")

# Find the required packages
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

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
target_link_libraries(
    server PRIVATE
    ws2_32 
    strmiids
    quartz
    gdiplus
    ole32
    uuid
)

# Client
add_executable(
    client 
    ./Client/client.cpp 
    ./Client/ClientSocket.hpp 
    ./Client/ClientSocket.cpp
    ./GmailAPI/GmailAPI.hpp
    ./GmailAPI/GmailAPI.cpp
    ./GmailAPI/Base64.hpp
    ./GmailAPI/Base64.cpp
    ./GmailAPI/OAuthManager.hpp
    ./GmailAPI/OAuthManager.cpp
    ./GmailAPI/HTMLGenerator.hpp
    ./GmailAPI/HTMLGenerator.cpp
)

target_link_libraries(
    client PRIVATE 
    ws2_32 
    nlohmann_json::nlohmann_json 
    CURL::libcurl 
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    add_custom_command(TARGET server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
        "${CMAKE_SOURCE_DIR}/build/Debug")

    add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
        "${CMAKE_SOURCE_DIR}/build/Debug")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    
    add_custom_command(TARGET server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
        "${CMAKE_SOURCE_DIR}/build")

    add_custom_command(TARGET client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/build"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
        "${CMAKE_SOURCE_DIR}/build")

endif()