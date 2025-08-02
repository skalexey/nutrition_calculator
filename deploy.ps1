# Check if the script is running with elevated privileges
function Test-Admin {
	$currentUser = New-Object Security.Principal.WindowsPrincipal $([Security.Principal.WindowsIdentity]::GetCurrent())
	$currentUser.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Prompt for elevation if not already running as administrator
if (-not (Test-Admin)) {
	Write-Host "This script needs to be run as administrator. Restarting with elevated privileges..."	$scriptPath = $MyInvocation.MyCommand.Path
	$scriptArguments = $MyInvocation.MyCommand.Definition
	Start-Process powershell.exe -Verb RunAs -ArgumentList "-File `"$scriptPath`" $scriptArguments"
	Exit
}

$executable_name = $args[0]
$build_dir = $args[1]
$deploy_dir = $args[2]
$build_type = $args[3]
$app_folder = if ($args.Length -gt 4) { $args[4] } else { "" }

Write-Host "Build type to deploy: $build_type"
Write-Host "Deploying $executable_name from '$build_dir' to '$deploy_dir'"

# Check if the directory already exists
if (-not (Test-Path -Path $deploy_dir -PathType Container)) {
    # If not, create the directory
    New-Item -ItemType Directory -Path $deploy_dir -Force
    Write-Host "Directory created: $deploy_dir"
} else {
    Write-Host "Deploy directory already exists: '$deploy_dir'"
}

# Check if the source directory exists
if (Test-Path -Path $build_dir -PathType Container) {
	# # Deploy Qt DLLs
	# $scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Definition
	# $batFilePath = Join-Path $scriptDirectory "deploy.bat"
	# $processStartInfo = @{
	# 	FilePath = $batFilePath
	# 	NoNewWindow = $true
	# 	Wait = $true
	# }
	# if ($args) {
	# 	$processStartInfo['ArgumentList'] = $args
	# }
	# Start-Process @processStartInfo
	# # Run windeployqt.exe to copy the required Qt DLLs
	# # $THIS_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
	# # $appLocation = Join-Path $build_dir ($executable_name + ".exe")
	# # $Env:QT_DIR\\msvc2019_64\\bin\\windeployqt.exe $appLocation --qmldir $THIS_DIR
	# if ($?) {
	# 	Write-Host "Qt DLLs copied from '$build_dir' to '$deploy_dir'"
	# } else {
	# 	Write-Host "Error: Copying Qt DLLs from '$build_dir' to '$deploy_dir' failed. See the log above ^^" -ForegroundColor Red
	# 	exit 1
	# }
	# Copy files from the source directory to the newly created directory
	Copy-Item -Path $build_dir\* -Destination $deploy_dir -Recurse -Force
	if ($?) {
		Write-Host "Files copied from '$build_dir' to '$deploy_dir'"
	} else {
		Write-Host "Error: Copying files from '$build_dir' to '$deploy_dir' failed. See the log above ^^" -ForegroundColor Red
		exit 2
	}
} else {
	Write-Host "Error: Build directory '$build_dir' does not exist. Please build the project first, or correct the path."
	exit 3
}

# Option: Add to PATH
Write-Host "Adding '$deploy_dir' to the User PATH environment variable..."
# Check if the directory is already in Path
$current_path = [Environment]::GetEnvironmentVariable('Path', [EnvironmentVariableTarget]::User)
if ($current_path -notlike "*$deploy_dir*") {
	# Append the directory to Path if not already present
	[Environment]::SetEnvironmentVariable('Path', "$current_path;$deploy_dir", [EnvironmentVariableTarget]::User)
	Write-Output "Added '$deploy_dir' to the User PATH environment variable."
} else {
	Write-Output "'$deploy_dir' is already in the User PATH environment variable."
}

# Option: Create a shortcut in the start menu
$program_path = If ($app_folder -eq "") { $executable_name } else { Join-Path $app_folder $executable_name }
$shortcut_path = $Env:APPDATA + "\Microsoft\Windows\Start Menu\Programs\" + $program_path + ".lnk"
$null = New-Item -ItemType Directory -Path ([System.IO.Path]::GetDirectoryName($shortcut_path)) -Force
$shell = New-Object -ComObject ("WScript.Shell")
$shortcut = $shell.CreateShortcut($shortcut_path)
$shortcut.TargetPath = Join-Path $deploy_dir ($executable_name + ".exe")
$shortcut.Save()
Write-Host "Shortcut '$shortcut_path' created in the start menu for: $($shortcut.TargetPath)"
