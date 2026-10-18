[CmdletBinding()]
param([switch]$Browser,[string]$EmsdkPath,[string]$JoltSource,[int]$Jobs=4)
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path (Join-Path $PSScriptRoot '../../..')).Path
$trackSources=(Join-Path $repoRoot 'src/release1/rules/puzzles.cpp')+';'+(Join-Path $repoRoot 'src/release1/rules/content.cpp')
$mode=if($Browser){'web'}else{'native'}
$buildPath=Join-Path $repoRoot "build/$mode-release1-p04-pinned"
$argsList=@('-S',(Join-Path $repoRoot 'tools/release1-track'),'-B',$buildPath,'-G','Ninja','-DPAWS_TRACK=rules',"-DPAWS_TRACK_SOURCES=$trackSources",'-DCMAKE_BUILD_TYPE=Release',"-DPAWS_TRACK_TEST=$(Join-Path $repoRoot 'tests/release1/rules/tests.cpp')")
if($JoltSource){$argsList+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSource"}
if($Browser){
    if(!$EmsdkPath){$EmsdkPath=Join-Path $repoRoot '.tools/emsdk'}
    $argsList+=@("-DPAWS_TRACK_BROWSER_MAIN=$(Join-Path $repoRoot 'src/release1/rules/debug_main.cpp')","-DPAWS_TRACK_WEB=$(Join-Path $repoRoot 'web/release1/rules')")
    & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @argsList
}else{& cmake @argsList}
if($LASTEXITCODE){throw 'Configure failed'}
$target=if($Browser){'release1_track_site'}else{'release1_track_tests'}
& cmake --build $buildPath --target $target --parallel $Jobs
if($LASTEXITCODE){throw 'Build failed'}
if(!$Browser){& ctest --test-dir $buildPath --output-on-failure;if($LASTEXITCODE){throw 'Tests failed'}}
