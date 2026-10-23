option(PAWS_BUILD_WINDOWS "Build the playable Windows Paws host" OFF)
if(NOT PAWS_BUILD_WINDOWS)
  return()
endif()
if(NOT TARGET WebEngine::Desktop)
  message(FATAL_ERROR "Paws Windows needs WEBENGINE_BUILD_WINDOWS=ON and the Windows SDK dependencies")
endif()
find_package(SDL3 3.4.18 CONFIG REQUIRED)
get_filename_component(_sdl_package "${SDL3_DIR}" DIRECTORY)
set(_audio_header "${CMAKE_BINARY_DIR}/generated/paws_native_audio.inc")
execute_process(COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/prepare-desktop-audio.py"
  --source "${PROJECT_SOURCE_DIR}/assets/runtime/audio" --header "${_audio_header}" COMMAND_ERROR_IS_FATAL ANY)
add_executable(paws_windows src/windows_main.cpp)
target_link_libraries(paws_windows PRIVATE paws_session WebEngine::Desktop SDL3::SDL3)
target_include_directories(paws_windows PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_compile_definitions(paws_windows PRIVATE PAWS_DAYTIME_SKY=1 PAWS_GUI_GALLERY=0 WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(paws_windows PRIVATE /utf-8 /W4)
set_target_properties(paws_windows PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/package")
add_custom_target(paws_windows_package ALL
  COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DSITE_DIR=$<TARGET_FILE_DIR:paws_windows>
    -P "${PROJECT_SOURCE_DIR}/cmake/StageAssets.cmake"
  COMMAND ${Python3_EXECUTABLE} "${PROJECT_SOURCE_DIR}/scripts/prepare-desktop-audio.py"
    --source "${PROJECT_SOURCE_DIR}/assets/runtime/audio" --header "${_audio_header}"
    --output "$<TARGET_FILE_DIR:paws_windows>/assets/audio/native"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL3::SDL3-shared>"
    "${WEBENGINE_DXC_DIR}/bin/x64/dxcompiler.dll" "${WEBENGINE_DXC_DIR}/bin/x64/dxil.dll"
    "$<TARGET_FILE_DIR:paws_windows>"
  COMMAND ${CMAKE_COMMAND} -DENGINE_SOURCE_DIR=${WEBENGINE_SOURCE_DIR} -DJOLT_SOURCE_DIR=${PAWS_JOLT_SOURCE_DIR}
    -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DSITE_DIR=$<TARGET_FILE_DIR:paws_windows>
    -P "${PROJECT_SOURCE_DIR}/cmake/CopyLicenses.cmake"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_sdl_package}/LICENSE.txt" "$<TARGET_FILE_DIR:paws_windows>/licenses/SDL3.txt"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_DXC_DIR}/LICENCE-MIT.txt" "$<TARGET_FILE_DIR:paws_windows>/licenses/DXC-MIT.txt"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_DXC_DIR}/LICENSE-LLVM.txt" "$<TARGET_FILE_DIR:paws_windows>/licenses/DXC-LLVM.txt"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_DXC_DIR}/LICENSE-MS.txt" "$<TARGET_FILE_DIR:paws_windows>/licenses/DXC-MS.txt"
  COMMAND ${CMAKE_COMMAND} -DENGINE_SOURCE_DIR=${WEBENGINE_SOURCE_DIR} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
    -DCONFIGURATION=$<CONFIG> -DEXECUTABLE=$<TARGET_FILE:paws_windows> -DPACKAGE_DIR=$<TARGET_FILE_DIR:paws_windows>
    -P "${PROJECT_SOURCE_DIR}/cmake/DesktopBuildInfo.cmake"
  DEPENDS paws_windows
  VERBATIM)
