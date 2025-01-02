param (
    [string]$oauth2TokenFilePath = "./scripts/oauth2.json",
    [string]$outputTokenFilePath = "./scripts/token.json",
    [string]$accountFilePath = "./scripts/account.json"
)

# Define the authorization URL
$json = Get-Content -Raw -Path $oauth2TokenFilePath | ConvertFrom-Json
$clientId = $json.installed.client_id
$clientSecret = $json.installed.client_secret

# Define multiple scopes
$scopes = @(
    "https://www.googleapis.com/auth/gmail.send",
    "https://www.googleapis.com/auth/gmail.readonly",
    "https://www.googleapis.com/auth/gmail.modify",
    "https://www.googleapis.com/auth/userinfo.email"
)

$scopeString = [string]::Join("%20", $scopes)

$authUrl = "https://accounts.google.com/o/oauth2/v2/auth?client_id=$clientId&scope=$scopeString&access_type=offline&response_type=code&redirect_uri=urn:ietf:wg:oauth:2.0:oob"

# Open the default browser and navigate to the authorization URL
Start-Process $authUrl

# Wait for user to log in and authorize
Write-Output "Please log in to your Google account and authorize the application."

# Prompt user to enter the authorization code
$authorization_code = Read-Host "Enter the authorization code"

# Request for access token
$body = "client_id=$clientId&client_secret=$clientSecret&redirect_uri=urn:ietf:wg:oauth:2.0:oob&code=$authorization_code&grant_type=authorization_code"

try {
    $response = Invoke-RestMethod -Uri 'https://www.googleapis.com/oauth2/v4/token' -ContentType 'application/x-www-form-urlencoded' -Method POST -Body $body
    $response | ConvertTo-Json | Out-File -FilePath $outputTokenFilePath -Encoding utf8
    Write-Output "Token information saved to " $outputTokenFilePath

    # Get user info
    $accessToken = $response.access_token
    $userInfo = Invoke-RestMethod -Uri 'https://www.googleapis.com/oauth2/v2/userinfo' -Headers @{Authorization = "Bearer $accessToken"}
    $userInfo | ConvertTo-Json | Out-File -FilePath $accountFilePath -Encoding utf8
    Write-Output "User information saved to " $accountFilePath
} catch {
    Write-Error "Failed to retrieve the access token."
    exit 1
}