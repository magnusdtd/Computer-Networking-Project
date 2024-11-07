cmake_minimum_required(VERSION 3.10)

project(RemoteDesktopWithGmailAPI)

# Use vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the prefix path for vcpkg
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows")

# Find the required packages
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# Server
add_executable(
    server 
    ./Server/server.cpp 
    ./Server/ServerSocket.hpp 
    ./Server/ServerSocket.cpp 
    ./WindowAPI/winAPI.cpp 
    ./WindowAPI/winAPI.hpp
)

target_link_libraries(server PRIVATE ws2_32)

add_custom_command(TARGET server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
    $<TARGET_FILE_DIR:server>)


# Client
add_executable(
    client 
    ./Client/client.cpp 
    ./Client/ClientSocket.hpp 
    ./Client/ClientSocket.cpp
)

target_link_libraries(client PRIVATE ws2_32 nlohmann_json::nlohmann_json CURL::libcurl OpenSSL::SSL)

add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
    $<TARGET_FILE_DIR:client>)

#===============================================================================
# Google Test
find_package(GTest REQUIRED)

add_executable(
    tests 
    ./Tests/test_main.cpp 
    ./Tests/winAPI_test.cpp
    ./WindowAPI/winAPI.hpp    
    ./WindowAPI/winAPI.cpp    
)

target_link_libraries(tests PRIVATE GTest::gtest GTest::gtest_main)

add_custom_command(TARGET tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/debug/bin"
    $<TARGET_FILE_DIR:tests>)