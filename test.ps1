Write-Output ""
Write-Output "Running the project test cases ..."
Write-Output ""
$serverProcess = Start-Process -FilePath "./build/Debug/tests.exe" -NoNewWindow -PassThru