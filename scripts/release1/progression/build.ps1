[CmdletBinding()]
param([switch]$Wasm,[string]$EmsdkPath,[string]$JoltSource,[int]$Jobs=4)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path (Join-Path $PSScriptRoot '../../..')).Path
python (Join-Path $PSScriptRoot 'prepare.py')
if($LASTEXITCODE){throw 'Preparation failed'}
$files=@('src/release1/progression/policy.cpp','src/release1/progression/codec.cpp','src/release1/progression/runtime/afternoon.cpp','src/release1/rules/puzzles.cpp','src/release1/rules/content.cpp')
$sources=($files | ForEach-Object {Join-Path $repo $_}) -join ';'
$mode=if($Wasm){'web'}else{'native'}
$build=Join-Path $repo "build/$mode-release1-p05-pinned"
$arguments=@('-S',(Join-Path $repo 'tools/release1-track'),'-B',$build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug','-DPAWS_TRACK=progression',"-DPAWS_TRACK_SOURCES=$sources","-DPAWS_TRACK_TEST=$(Join-Path $repo 'tests/release1/progression/tests.cpp')")
if($JoltSource){$arguments+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSource"}
if($Wasm){
    if(!$EmsdkPath){$EmsdkPath=Join-Path $repo '.tools/emsdk'}
    & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @arguments
}else{cmake @arguments}
if($LASTEXITCODE){throw 'Configure failed'}
$target=if($Wasm){'release1_track_compile'}else{'release1_track_tests'}
cmake --build $build --target $target --parallel $Jobs
if($LASTEXITCODE){throw 'Build failed'}
if(!$Wasm){ctest --test-dir $build --output-on-failure;if($LASTEXITCODE){throw 'Tests failed'}}
