param (
    [string]$buildType
)

if (-not $buildType) {
    Write-Output "Usage: .\setup_and_build.ps1 -buildType=<MSVC|GNU>"
    exit 1
}

# Check if vcpkg is already cloned
if (-not (Test-Path -Path "./vcpkg")) {
    Write-Output "Cloning vcpkg repository..."
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    Write-Output "Bootstrapping vcpkg..."
    .\bootstrap-vcpkg.bat
    Write-Output "Integrating vcpkg..."
    .\vcpkg.exe integrate install
    cd ..
} else {
    Write-Output "vcpkg is already cloned."
}

# Check if dependencies are already installed
$dependencies = @("nlohmann-json", "curl", "openssl")
$installed = $true
foreach ($dep in $dependencies) {
    if (-not (Test-Path -Path "./vcpkg/installed/x64-windows/$dep")) {
        $installed = $false
        break
    }
}

if (-not $installed) {
    Write-Output "Installing dependencies..."
    cd vcpkg
    .\vcpkg.exe install nlohmann-json curl openssl
    cd ..
} else {
    Write-Output "Dependencies are already installed."
}

# Clear CMake cache
Write-Output "Clearing CMake cache..."
if (Test-Path -Path "./build/CMakeCache.txt") {
    Remove-Item -Path "./build/CMakeCache.txt" -Force
}
if (Test-Path -Path "./build/CMakeFiles") {
    Remove-Item -Path "./build/CMakeFiles" -Recurse -Force
}

# Configure the project using CMake
Write-Output "Configuring project with CMake..."
if ($buildType -eq "MSVC") {
    cmake -S . -B build
} elseif ($buildType -eq "GNU") {
    cmake -S . -B build -G "MinGW Makefiles"
} else {
    Write-Error "Invalid build type specified. Use MSVC or GNU."
    exit 1
}

# Build the project
Write-Output "Building project..."
cmake --build build --target server client
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit $LASTEXITCODE
}
