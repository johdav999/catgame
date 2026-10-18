[CmdletBinding()]
param([switch]$Browser,[string]$EmsdkPath,[string]$JoltSource,[int]$Jobs=4)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path "$PSScriptRoot/../../..").Path
python "$PSScriptRoot/prepare.py"
if($LASTEXITCODE){throw 'GUI fixture preparation failed'}
$files=@('src/release1/gui/views.cpp','src/release1/gui/dialogue.cpp','src/release1/gui/runtime/paws_ui.cpp','src/release1/progression/policy.cpp','src/release1/progression/codec.cpp','src/release1/progression/runtime/afternoon.cpp','src/release1/rules/puzzles.cpp','src/release1/rules/content.cpp')
$sources=($files | ForEach-Object {Join-Path $repo $_}) -join ';'
$mode=if($Browser){'web'}else{'native'}
$build=Join-Path $repo "build/$mode-release1-p07-pinned-release"
$arguments=@('-S',"$repo/tools/release1-track",'-B',$build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPAWS_TRACK=gui',"-DPAWS_TRACK_SOURCES=$sources","-DPAWS_TRACK_TEST=$repo/tests/release1/gui/tests.cpp")
if($JoltSource){$arguments+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSource"}
if($Browser){
    if(!$EmsdkPath){$EmsdkPath=Join-Path $repo '.tools/emsdk'}
    $arguments+=@("-DPAWS_TRACK_BROWSER_MAIN=$repo/src/release1/gui/gallery_main.cpp","-DPAWS_TRACK_WEB=$repo/web/release1/gui")
    & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @arguments
}else{
    if(!(Get-Command c++ -ErrorAction SilentlyContinue)){
        $cache=Get-Content "$repo/build/native/CMakeCache.txt"
        $compiler=($cache | Select-String '^CMAKE_CXX_COMPILER:.*?=(.*)$').Matches.Groups[1].Value
        $env:PATH="$(Split-Path $compiler);$env:PATH"
    }
    cmake @arguments
}
if($LASTEXITCODE){throw 'GUI configure failed'}
$target=if($Browser){'release1_track_site'}else{'release1_track_tests'}
cmake --build $build --target $target --parallel $Jobs
if($LASTEXITCODE){throw 'GUI build failed'}
if(!$Browser){
    $compilerLine=Get-Content "$build/CMakeCache.txt" | Select-String '^CMAKE_CXX_COMPILER:.*?='
    $compilerPath=$compilerLine.Line -replace '^CMAKE_CXX_COMPILER:.*?=',''
    $env:PATH="$(Split-Path $compilerPath);$env:PATH"
    ctest --test-dir $build --output-on-failure
    if($LASTEXITCODE){throw 'GUI tests failed'}
}
