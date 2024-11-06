$oauth2TokenFileName = "oauth2.json"

# Read clientId and clientSecret from json file
$json = Get-Content -Raw -Path $oauth2TokenFileName | ConvertFrom-Json
$clientId = $json.installed.client_id
$clientSecret = $json.installed.client_secret

# Define multiple scopes
$scopes = @(
    "https://www.googleapis.com/auth/gmail.send",
    "https://www.googleapis.com/auth/gmail.readonly"
)

$scopeString = [string]::Join("%20", $scopes)

Start-Process "https://accounts.google.com/o/oauth2/v2/auth?client_id=$clientId&scope=$scopeString&access_type=offline&response_type=code&redirect_uri=urn:ietf:wg:oauth:2.0:oob"    

$code = Read-Host "Please enter the code"

$body = "client_id=$clientId&client_secret=$clientSecret&redirect_uri=urn:ietf:wg:oauth:2.0:oob&code=$code&grant_type=authorization_code"

$response = Invoke-WebRequest https://www.googleapis.com/oauth2/v4/token -ContentType application/x-www-form-urlencoded -Method POST -Body $body

$response.Content | Out-File -FilePath "token.json" -Encoding utf8

Write-Output "Token information saved to token.json"