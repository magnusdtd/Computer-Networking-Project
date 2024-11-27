Write-Output "Building test cases ..."
Write-Output ""
cmake --build build --target tests
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit $LASTEXITCODE
}
Write-Output ""
Write-Output "Running the project test cases ..."
Write-Output ""
$serverProcess = Start-Process -FilePath "./build/Debug/tests.exe" -NoNewWindow -PassThru