param (
    [string]$filePath
)

Write-Output "Running project ..."
Write-Output ""

# Run cmake build command
Write-Output "Building project ..."
Write-Output ""
cmake --build build --target server client
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit $LASTEXITCODE
}

# Run the server executable and measure its execution time
Write-Output ""
Write-Output "Running the server executable ..."
Write-Output ""
$serverProcess = Start-Process -FilePath "./build/Debug/server.exe" -NoNewWindow -PassThru

Start-Sleep -Seconds 2 # Give the server some time to start

# Run the client executable in a new terminal
Write-Output ""
Write-Output "Running the client executable in a new terminal ..."
Write-Output ""
$clientProcess = Start-Process -FilePath "powershell.exe" -ArgumentList "-NoExit", "-Command", "./build/Debug/client.exe" -PassThru

# Wait for the client process to exit
$clientProcess.WaitForExit()
if ($LASTEXITCODE -ne 0) {
    Write-Error "Client run failed."
    # Optionally, you can stop the server process if the client fails
    Stop-Process -Id $serverProcess.Id
    exit $LASTEXITCODE
}

if ($LASTEXITCODE -ne 0) {
    Write-Error "Server run failed."
    exit $LASTEXITCODE
}