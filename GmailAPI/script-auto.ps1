# Install Selenium module if not already installed
if (-not (Get-Module -ListAvailable -Name Selenium)) {
    Install-Module -Name Selenium -Scope CurrentUser -Force -SkipPublisherCheck
}

# Import Selenium module
Import-Module Selenium

$oauth2TokenFileName = "oauth2.json"
$accountFileName = "account.json"

# Read clientId and clientSecret from json file
$json = Get-Content -Raw -Path $oauth2TokenFileName | ConvertFrom-Json
$clientId = $json.installed.client_id
$clientSecret = $json.installed.client_secret

$json = Get-Content -Raw -Path $accountFileName | ConvertFrom-Json
$userName = $json.userName
$password = $json.password

# Define multiple scopes
$scopes = @(
    "https://www.googleapis.com/auth/gmail.send",
    "https://www.googleapis.com/auth/gmail.readonly",
    "https://www.googleapis.com/auth/gmail.modify"
)

$scopeString = [string]::Join("%20", $scopes)

$authUrl = "https://accounts.google.com/o/oauth2/v2/auth?client_id=$clientId&scope=$scopeString&access_type=offline&response_type=code&redirect_uri=urn:ietf:wg:oauth:2.0:oob"

# Start Selenium WebDriver with Firefox
$driver = Start-SeFirefox

# Navigate to the authorization URL
$driver.Navigate().GoToUrl($authUrl)


# Fill in the email
Start-Sleep -Seconds 3
$emailElement = $driver.FindElementById("identifierId").SendKeys($userName)
# Click the "Next" button
$nextButton = $driver.FindElementById("identifierNext").Click()


# Fill in the password
Start-Sleep -Seconds 3
$passwordElement = $driver.FindElementByName("Passwd").SendKeys($password)
# Click the "Next" button
$nextButton = $driver.FindElementById("passwordNext").Click()


# Click continue button
Start-Sleep -Seconds 5
$continueButton = $driver.FindElementByXPath('//button[@class="VfPpkd-LgbsSe VfPpkd-LgbsSe-OWXEXe-dgl2Hf ksBjEc lKxP2d LQeN7 BqKGqe eR0mzb TrZEUc lw1w4b"]').Click();


#Click select all and continue
Start-Sleep -Seconds 3
$selectAllButton = $driver.FindElementById('i1').Click();
$continueButton = $driver.FindElementByXPath('//div[@class="XjS9D TrZEUc zwnjJd JfDd"]').Click();


# # Get the authorization code from the page
$codeElement = $driver.FindElementByXPath('//textarea[@class="fD1Pid"]')
$authorization_code = $codeElement.Text

Write-Output "The authorization code: $authorization_code"
# Stop the WebDriver
Stop-SeDriver -Driver $driver


# Request for access token
$body = "client_id=$clientId&client_secret=$clientSecret&redirect_uri=urn:ietf:wg:oauth:2.0:oob&code=$authorization_code&grant_type=authorization_code"

try {
    $response = Invoke-RestMethod -Uri 'https://www.googleapis.com/oauth2/v4/token' -ContentType 'application/x-www-form-urlencoded' -Method POST -Body $body
    $response | ConvertTo-Json | Out-File -FilePath "token.json" -Encoding utf8
    Write-Output "Token information saved to token.json"
} catch {
    Write-Error "Failed to retrieve the access token."
    exit 1
}