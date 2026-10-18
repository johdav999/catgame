[CmdletBinding()]
param(
    [string]$SiteDirectory,
    [ValidateRange(1,65535)][int]$Port = 8080,
    [ValidateRange(1,100)][int]$PortAttempts = 100,
    [switch]$Restart
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
if (-not $SiteDirectory) {
    $buildRoot = Join-Path $repoRoot 'build'
    $candidates = if (Test-Path -LiteralPath $buildRoot -PathType Container) {
        Get-ChildItem -LiteralPath $buildRoot -Directory | ForEach-Object {
            $site = Join-Path $_.FullName 'site'
            if ((Test-Path -LiteralPath (Join-Path $site 'index.html') -PathType Leaf) -and
                (Test-Path -LiteralPath (Join-Path $site 'paws.js') -PathType Leaf) -and
                (Test-Path -LiteralPath (Join-Path $site 'paws.wasm') -PathType Leaf)) {
                Get-Item -LiteralPath (Join-Path $site 'paws.wasm')
            }
        }
    }
    $latest = $candidates | Sort-Object LastWriteTimeUtc, FullName -Descending | Select-Object -First 1
    if (-not $latest) {
        throw 'No built game found under build/*/site. Run ./scripts/build.ps1 -Pinned, or pass -SiteDirectory for a custom build.'
    }
    $SiteDirectory = $latest.DirectoryName
}
foreach ($file in @('index.html', 'paws.js', 'paws.wasm')) {
    if (-not (Test-Path -LiteralPath (Join-Path $SiteDirectory $file) -PathType Leaf)) {
        throw "Incomplete game site: no $file in $SiteDirectory. Build the game first or choose another -SiteDirectory."
    }
}
$SiteDirectory = (Resolve-Path -LiteralPath $SiteDirectory).Path
$serverScript = Join-Path $PSScriptRoot 'serve-site.py'
if ($Restart) {
    # Switch only this repository's development server, preserving other apps.
    $listeners = @(Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue)
    $serverProcessIds = @($listeners.OwningProcess | Select-Object -Unique)
    foreach ($serverProcessId in $serverProcessIds) {
        $serverProcess = Get-CimInstance Win32_Process -Filter "ProcessId = $serverProcessId"
        $scriptPattern = '(?i)(?:^|\s)"?' + [regex]::Escape($serverScript) + '"?(?:\s|$)'
        $portPattern = '(?:^|\s)--port\s+' + $Port + '(?:\s|$)'
        if (-not $serverProcess -or $serverProcess.Name -notmatch '^python(?:\d+(?:\.\d+)?)?\.exe$' -or
            $serverProcess.CommandLine -notmatch $scriptPattern -or
            $serverProcess.CommandLine -notmatch $portPattern) {
            throw "Port $Port belongs to another process. It was not stopped. Choose another -Port."
        }
    }
    foreach ($serverProcessId in $serverProcessIds) {
        Write-Host "Stopping previous Paws server on port $Port (PID $serverProcessId)."
        Stop-Process -Id $serverProcessId -ErrorAction Stop
        Wait-Process -Id $serverProcessId -Timeout 10 -ErrorAction SilentlyContinue
    }
}
Write-Host "Serving: $SiteDirectory"
Write-Host 'Selecting an available port (Ctrl+C to stop).'
& python $serverScript --port $Port --port-attempts $PortAttempts --directory $SiteDirectory
if ($LASTEXITCODE -ne 0) { throw 'Server stopped with an error.' }
