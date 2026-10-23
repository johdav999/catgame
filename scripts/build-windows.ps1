[CmdletBinding()]
param(
  [ValidateSet('Debug','Release')][string]$Configuration='Release',
  [string]$WebEngineSourceDir,
  [string]$WindowsDependenciesDirectory,
  [string]$BuildDirectory,
  [ValidateRange(1,64)][int]$Jobs=4,
  [switch]$Pinned
)
$ErrorActionPreference='Stop'
$repoRoot=Split-Path -Parent $PSScriptRoot
$settingsFile=Join-Path $repoRoot 'developer-settings.json'
if(Test-Path $settingsFile){
  $settings=Get-Content $settingsFile -Raw | ConvertFrom-Json
  if(-not $Pinned -and -not $WebEngineSourceDir){$WebEngineSourceDir=$settings.WebEngineSourceDir}
  if(-not $WindowsDependenciesDirectory){$WindowsDependenciesDirectory=$settings.WindowsDependenciesDirectory}
}
if($Pinned -and $WebEngineSourceDir){throw 'Use -Pinned or -WebEngineSourceDir.'}
if(-not $WebEngineSourceDir){$WebEngineSourceDir=Join-Path $repoRoot 'external/WebEngine'}
if(-not $WindowsDependenciesDirectory){$WindowsDependenciesDirectory=Join-Path $WebEngineSourceDir 'build/dependencies/windows'}
if(-not $BuildDirectory){
  $mode=if($Pinned -or $WebEngineSourceDir -eq (Join-Path $repoRoot 'external/WebEngine')){'pinned'}else{'local'}
  $BuildDirectory=Join-Path $repoRoot "build/windows-$mode-$($Configuration.ToLowerInvariant())"
}
$sdl=Join-Path $WindowsDependenciesDirectory 'sdl/SDL3-3.4.18/cmake'
$dawn=Join-Path $WindowsDependenciesDirectory 'dawn/Dawn-67d4d6e64b0755d638f76549794692badfdbc8ab-windows-latest-Release/lib/cmake/Dawn'
$dxc=Join-Path $WindowsDependenciesDirectory 'dxc'
foreach($folder in @($sdl,$dawn,$dxc)){if(-not(Test-Path $folder)){throw "Missing SDK: $folder. Run the selected engine's scripts/setup-windows.ps1 or pass -WindowsDependenciesDirectory."}}
if(-not(Get-Command ffmpeg -ErrorAction SilentlyContinue)){throw 'ffmpeg is required to package the reviewed MP3 music as native PCM WAV.'}
& cmake -S $repoRoot -B $BuildDirectory -A x64 "-DWEBENGINE_SOURCE_DIR=$WebEngineSourceDir" -DWEBENGINE_BUILD_WINDOWS=ON -DPAWS_BUILD_WINDOWS=ON "-DSDL3_DIR=$sdl" "-DDawn_DIR=$dawn" "-DWEBENGINE_DXC_DIR=$dxc"
if($LASTEXITCODE -ne 0){throw 'Windows configuration failed.'}
& cmake --build $BuildDirectory --config $Configuration --target paws_windows_package --parallel $Jobs
if($LASTEXITCODE -ne 0){throw 'Windows build failed.'}
Write-Host "Paws package: $BuildDirectory/package/$Configuration/paws_windows.exe"
