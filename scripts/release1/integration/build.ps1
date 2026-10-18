param([switch]$Native,[int]$Jobs=4,[string]$EmsdkPath,[string]$JoltSourceDir)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path "$PSScriptRoot/../../..").Path
$pin=git -C "$repo/external/WebEngine" rev-parse HEAD
$expectedPin=(Get-Content (Join-Path $repo 'docs/release1/contract.json') -Raw | ConvertFrom-Json).enginePin
if($pin -ne $expectedPin){throw "The production build requires engine pin $expectedPin"}
if(git -C "$repo/external/WebEngine" status --porcelain){throw 'The pinned engine checkout must be clean'}
$directory=Join-Path $repo $(if($Native){'build/native-release1-p09'}else{'build/web-release1-p09-pinned-release'})
$arguments=@('-S',$repo,'-B',$directory,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPAWS_GUI_GALLERY=OFF','-DPAWS_ASSET_PREVIEWS=OFF')
if($JoltSourceDir){$arguments+="-DFETCHCONTENT_SOURCE_DIR_JOLT=$JoltSourceDir"}
if($Native){cmake @arguments}
else{
 if(!$EmsdkPath){$EmsdkPath=Join-Path $repo '.tools/emsdk'}
 & (Join-Path $EmsdkPath 'upstream/emscripten/emcmake.exe') cmake @arguments
}
if($LASTEXITCODE){throw 'Production configure failed'}
if($Native){cmake --build $directory --parallel $Jobs}
else{cmake --build $directory --target paws_site --parallel $Jobs}
if($LASTEXITCODE){throw 'Production build failed'}
if(!$Native){python "$PSScriptRoot/assets.py" --site "$directory/site" --report "$repo/build/evidence/release1/prompt-09/assets.json";if($LASTEXITCODE){throw 'Production stable-asset audit failed'}}
