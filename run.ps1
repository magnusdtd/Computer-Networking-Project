param (
    [string]$filePath
)

Write-Output "Running project ..."
Write-Output ""

# Run cmake build command
Write-Output "Building project ..."
Write-Output ""
cmake --build build
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

# Measure the execution time of the server process
$executionTime = Measure-Command {
    $serverProcess.WaitForExit()
}
if ($LASTEXITCODE -ne 0) {
    Write-Error "Server run failed."
    exit $LASTEXITCODE
}

$days = $executionTime.Days
$hours = $executionTime.Hours
$minutes = $executionTime.Minutes
$seconds = $executionTime.Seconds
$milliseconds = $executionTime.Milliseconds

Write-Output ""
if ($filePath) {
    $content = Get-Content -Path $filePath -Raw
    $charCount = $content.Length
    $fileSizeBytes = (Get-Item $filePath).Length

    $fileSizeKB = [math]::Round($fileSizeBytes / 1KB, 2)
    $fileSizeMB = [math]::Round($fileSizeBytes / 1MB, 2)
    $fileSizeGB = [math]::Round($fileSizeBytes / 1GB, 2)

    Write-Output "==============================================================="
    Write-Output "The number of characters in the file is: $charCount"
    Write-Output "The size of the file is: $fileSizeBytes bytes ($fileSizeKB KB, $fileSizeMB MB, $fileSizeGB GB)"
    Write-Output "==============================================================="
} else {
    Write-Output "==============================================================="
    Write-Output "Execution Time: $days days, $hours hours, $minutes minutes, $seconds seconds, $milliseconds milliseconds"
    Write-Output "==============================================================="
}