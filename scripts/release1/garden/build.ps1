param([switch]$Native,[int]$Jobs=4,[string]$EmsdkPath,[string]$JoltSourceDir)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path "$PSScriptRoot/../../..").Path
python "$PSScriptRoot/prepare.py"
if($LASTEXITCODE){throw 'Garden input verification failed'}
$files=@('src/release1/garden/presentation.cpp','src/release1/garden/contact_plans.cpp','src/release1/props/prop_assets.cpp','src/release1/cats/cat_assets.cpp','src/release1/progression/policy.cpp','src/release1/progression/codec.cpp','src/release1/progression/runtime/afternoon.cpp','src/release1/rules/puzzles.cpp','src/release1/rules/content.cpp')
$sources=($files|ForEach-Object{Join-Path $repo $_}) -join ';'
$directory=Join-Path $repo $(if($Native){'build/native-release1-p06'}else{'build/web-release1-p06-pinned-release'})
$arguments=@('-S',"$repo/tools/release1-track",'-B',$directory,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPAWS_TRACK=garden',"-DPAWS_TRACK_SOURCES=$sources")
if($JoltSourceDir){$arguments+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSourceDir"}
if($Native){$arguments+="-DPAWS_TRACK_TEST=$repo/tests/release1/garden/tests.cpp";cmake @arguments}
else{
 if(!$EmsdkPath){$EmsdkPath=Join-Path $repo '.tools/emsdk'}
 $arguments+=@("-DPAWS_TRACK_BROWSER_MAIN=$repo/src/release1/garden/preview_main.cpp","-DPAWS_TRACK_WEB=$repo/web/release1/garden")
 & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @arguments
}
if($LASTEXITCODE){throw 'Garden configure failed'}
$target=if($Native){'release1_track_tests'}else{'release1_track_site'}
cmake --build $directory --target $target --parallel $Jobs
if($LASTEXITCODE){throw 'Garden build failed'}
if(!$Native){
 # Reuse the existing coordinator in a test-owned IndexedDB namespace. This
 # replacement is solely in the isolated staged shell, never the game's source.
 $saves=(Get-Content "$repo/web/saves.js" -Raw).Replace('paws-afternoon','paws-release1-garden-harness')
 Set-Content -LiteralPath "$directory/site/saves.js" -Value $saves -Encoding utf8
}
