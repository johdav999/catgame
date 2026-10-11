[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [string]$WebEngineSourceDir,
    [string]$EmsdkPath,
    [string]$BuildDirectory,
    [ValidateRange(1,64)][int]$Jobs = 2,
    [switch]$Pinned
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$settingsFile = Join-Path $repoRoot 'developer-settings.json'
if (-not $Pinned -and (Test-Path $settingsFile)) {
    $settings = Get-Content $settingsFile -Raw | ConvertFrom-Json
    if (-not $WebEngineSourceDir) { $WebEngineSourceDir = $settings.WebEngineSourceDir }
    if (-not $EmsdkPath) { $EmsdkPath = $settings.EmsdkPath }
}
if ($Pinned -and $WebEngineSourceDir) { throw 'Use either -Pinned or -WebEngineSourceDir.' }
if (-not $WebEngineSourceDir) { $WebEngineSourceDir = Join-Path $repoRoot 'external/WebEngine' }
if (-not $EmsdkPath) { $EmsdkPath = Join-Path $repoRoot '.tools/emsdk' }
if (-not $BuildDirectory) {
    $mode = if ($Pinned) { 'pinned' } elseif ($WebEngineSourceDir -eq (Join-Path $repoRoot 'external/WebEngine')) { 'pinned' } else { 'local' }
    $BuildDirectory = Join-Path $repoRoot "build/web-$mode-$($Configuration.ToLowerInvariant())"
}
$emccPath = Join-Path $EmsdkPath 'upstream/emscripten/emcc.exe'
$emcmakePath = Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe'
if (-not (Test-Path $emccPath)) { throw 'Run ./scripts/setup.ps1 or pass -EmsdkPath with an installed SDK.' }
$expected = (Get-Content (Join-Path $repoRoot '.emscripten-version') -Raw).Trim()
$versionOutput = & $emccPath --version
if ($LASTEXITCODE -ne 0 -or ($versionOutput -join "`n") -notmatch [regex]::Escape($expected)) {
    throw "Expected Emscripten $expected at $EmsdkPath."
}
& $emcmakePath cmake -S $repoRoot -B $BuildDirectory -G Ninja "-DCMAKE_BUILD_TYPE=$Configuration" "-DWEBENGINE_SOURCE_DIR=$WebEngineSourceDir"
if ($LASTEXITCODE -ne 0) { throw 'Paws CMake configuration failed.' }
& cmake --build $BuildDirectory --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw 'Paws build failed.' }
Write-Host "Paws ready in $BuildDirectory/site"
Write-Host "Serve with: ./scripts/serve.ps1 -SiteDirectory '$BuildDirectory/site'"
