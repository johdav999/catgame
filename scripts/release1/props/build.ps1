param([switch]$Native,[int]$Jobs=4,[string]$EmsdkPath,[string]$JoltSourceDir)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path "$PSScriptRoot/../../..").Path
$trackBuild=Join-Path $repo $(if($Native){'build/native-release1-p03'}else{'build/web-release1-p03-pinned-release'})
$argsList=@('-S',"$repo/tools/release1-track",'-B',$trackBuild,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPAWS_TRACK=props',"-DPAWS_TRACK_SOURCES=$repo/src/release1/props/prop_assets.cpp")
if($JoltSourceDir){$argsList+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSourceDir"}
if(-not $EmsdkPath){$EmsdkPath=Join-Path $repo '.tools/emsdk'}
if($Native){
  $argsList+="-DPAWS_TRACK_TEST=$repo/tests/release1/props/prop_tests.cpp"
  & cmake @argsList
}else{
  $argsList+=@("-DPAWS_TRACK_BROWSER_MAIN=$repo/src/release1/props/preview_main.cpp","-DPAWS_TRACK_WEB=$repo/web/release1/props")
  & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @argsList
}
if($LASTEXITCODE -ne 0){throw 'Prompt 3 configure failed'}
$target=if($Native){'release1_track_tests'}else{'release1_track_site'}
& cmake --build $trackBuild --target $target --parallel $Jobs
if($LASTEXITCODE -ne 0){throw 'Prompt 3 build failed'}
