# Socket Template Project

This project is a template for creating socket-based client-server applications in C++. It includes both client and server implementations, along with necessary build configurations.

## Files and Directories

- `.gitignore`: Specifies files and directories to be ignored by Git.
- `.vscode/`: Contains Visual Studio Code configuration files.
  - `c_cpp_properties.json`: Configuration for C/C++ IntelliSense.
  - `launch.json`: Configuration for debugging.
  - `settings.json`: User and workspace settings.
  - `tasks.json`: Configuration for task runner.
- `build/`: Directory for build output.
- `Client/`: Directory for client source files.
  - `client.cpp`: Main source file for the client.
- `CMakeLists.txt`: CMake build configuration file.
- `GmailAPI/`: Directory for Gmail API related files.
- `image/`: Directory for image assets.
- `Readme.md`: Project documentation file.
- `run.ps1`: PowerShell script to build and run the project.
- `Server/`: Directory for server source files.
- `vcpkg/`: Directory for vcpkg package manager.
- `WindowAPI/`: Directory for Windows API related files.

## Building the Project

To build the project, follow these steps:

1. Install [CMake](https://cmake.org/).
2. Install [vcpkg](https://github.com/microsoft/vcpkg).
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg; 
.\bootstrap-vcpkg.bat
.\vcpkg.exe integrate install
```
3. Install dependencies
```
cd vckpg
.\vcpkg install nlohmann-json curl openssl 
```
4. Configure the project using CMake:
    ```sh
    cmake -S . -B build
    ```
5. Build the project:
    ```sh
    cmake --build build
    ```

## Running the Applications

After building the project, you can run the client and server applications:

- To run the server:
    ```sh
    ./build/Debug/server.exe
    ```

- To run the client:
    ```sh
    ./build/Debug/client.exe
    ```
An alternative way to run server and client in one command

```
.\run.ps1
```

## Contact

For any questions or issues, please open an issue on the GitHub repository.