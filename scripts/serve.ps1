[CmdletBinding()]
param(
    [string]$SiteDirectory,
    [ValidateRange(1,65535)][int]$Port = 8080
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
Write-Host "Serving: $SiteDirectory"
Write-Host "Paws: http://localhost:$Port (Ctrl+C to stop)"
& python (Join-Path $PSScriptRoot 'serve-site.py') --port $Port --directory $SiteDirectory
if ($LASTEXITCODE -ne 0) { throw 'Server stopped with an error.' }
