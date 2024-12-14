# Remote Desktop with Gmail API

This project is a socket-based client-server application in C++. It includes both client and server implementations, along with necessary build configurations. The project also integrates with the Gmail API for email functionalities.

This project is built for **Windows**.

## Requirements
- [Git](https://git-scm.com/)
- Compiler: [GNU 13.1.0 or higher](https://gcc.gnu.org/) / [MSBuild from Visual Studio](https://visualstudio.microsoft.com/)
- [CMake](https://cmake.org/)

You need to install all of these.

## Files and Directories
- `build/`: Directory for build output.
- `Client/`: Directory for client source files.
  - `client.cpp`: Main source file for the client.
  - `ClientSocket.hpp`: Header file for the client socket class.
  - `ClientSocket.cpp`: Implementation file for the client socket class.
- `CMakeLists.txt`: CMake build configuration file.
- `GmailAPI/`: Directory for Gmail API related files.
  - `Base64.hpp`: Header file for Base64 encoding/decoding.
  - `Base64.cpp`: Implementation file for Base64 encoding/decoding.
  - `GmailAPI.hpp`: Header file for Gmail API class.
  - `GmailAPI.cpp`: Implementation file for Gmail API class.
  - `OAuthManager.hpp`: Header file for OAuth manager class.
  - `OAuthManager.cpp`: Implementation file for OAuth manager class.
  - `HTMLGenerator.hpp`: Header file for HTML Generator function.
  - `HTMLGenerator.cpp`: Implementation file for HTML Generator function.
  - `User.hpp`: Header file for User class.
- `img/`: Directory for image assets.
- `output-client/`: Directory for client output files.
- `output-server/`: Directory for server output files.
- `Readme.md`: Project documentation file.
- `run.ps1`: PowerShell script to build and run the project.
- `setup_and_build.ps1`: PowerShell script to install vcpkg, dependencies, and build the project.
- `test.ps1`: PowerShell script to build and run the test cases.
- `Server/`: Directory for server source files.
  - `server.cpp`: Main source file for the server.
  - `ServerSocket.hpp`: Header file for the server socket class.
  - `ServerSocket.cpp`: Implementation file for the server socket class.
- `WindowAPI/`: Directory for Windows API related files.
  - `FileOperations.hpp`: Header file for file operations.
  - `FileOperations.cpp`: Implementation file for file operations.
  - `Keylogger.hpp`: Header file for keylogger.
  - `Keylogger.cpp`: Implementation file for keylogger.
  - `KeyboardDisabler.hpp`: Header file for keyboard disabler.
  - `KeyboardDisabler.cpp`: Implementation file for keyboard disabler.
  - `MyUtility.hpp`: Header file for utility functions.
  - `MyUtility.cpp`: Implementation file for utility functions.
  - `ProcessOperations.hpp`: Header file for process operations.
  - `ProcessOperations.cpp`: Implementation file for process operations.
  - `SystemOperations.hpp`: Header file for system operations.
  - `SystemOperations.cpp`: Implementation file for system operations.
  - `VideoRecorder.hpp`: Header file for video recorder.
  - `VideoRecorder.cpp`: Implementation file for video recorder.

## Additional files for build

- `oauth2.json`: Your OAuth 2.0 credentials, you can get it in the Google API Console.
- `account.json`: Store your admin email.

```json
{
    "userName": "<your-admin-email>",
    "password": "<password>"
}
```
Place these files in the `./GmailAPI/` folder.

## UML Diagram

![alt text](./img/UML-diagram.png "UML diagram")

## Build and run the Project

### Build manually
To build the project, follow these steps:

1. Install [CMake](https://cmake.org/).
2. Install [vcpkg](https://github.com/microsoft/vcpkg).
    ```sh
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg.exe integrate install
    ```
3. Install dependencies
    ```sh
    .\vcpkg.exe install nlohmann-json curl openssl
    cd ..
    ```
4. Configure the project using CMake

    If using MSBuild (Visual Studio)

    ```sh
    cmake -S . -B build
    ```
    If using MinGW Makefiles

    ```sh
    cmake -S . -B build -G "MinGW Makefiles"
    ```

### Build automatically
  Just run the setup and build script

  ```sh
  .\setup_and_build.ps1 -buildType=<MSVC|GNU>
  ```
  Example:
   ```sh
  .\setup_and_build.ps1 GNU
  ```

### Run this project
  Run the project:
  ```sh
  .\run.ps1 -buildType=<MSVC|GNU>
  ```
  Example:
   ```sh
  .\run.ps1 GNU
  ```

## Contact

For any questions or issues, please open an issue on the GitHub repository.