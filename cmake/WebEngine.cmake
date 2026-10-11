# Compatibility packaging for the published engine revision. Keep all engine
# sources in WebEngine; this file only establishes consumer build boundaries.
# Remove the fallback when upstream publishes an engine_browser target.
function(paws_add_webengine)
  # Scope these to the dependency; Paws' own CTest option remains independent.
  set(BUILD_TESTING OFF)
  set(WEBENGINE_BUILD_DEMO OFF)
  set(WEBENGINE_BUILD_PERSISTENCE_WASM_TESTS OFF)
  set(WEBENGINE_BUILD_MOVEMENT_WASM_TESTS OFF)
  add_subdirectory("${WEBENGINE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/engine" EXCLUDE_FROM_ALL)
  include(FetchContent)
  FetchContent_GetProperties(jolt SOURCE_DIR paws_jolt_source)
  set(PAWS_JOLT_SOURCE_DIR "${paws_jolt_source}" PARENT_SCOPE)
  if(NOT TARGET WebEngine::Core)
    add_library(WebEngine::Core ALIAS engine_core)
  endif()
  if(NOT EMSCRIPTEN)
    return()
  endif()
  if(TARGET WebEngine::Browser)
    return()
  endif()
  if(TARGET engine_browser)
    add_library(WebEngine::Browser ALIAS engine_browser)
    return()
  endif()

  set(renderer_sources "${WEBENGINE_SOURCE_DIR}/src/renderer.cpp"
    "${WEBENGINE_SOURCE_DIR}/src/gui_renderer.cpp")
  # The adjacent development checkout currently splits the backend facade.
  foreach(backend webgpu_renderer webgl_renderer)
    if(EXISTS "${WEBENGINE_SOURCE_DIR}/src/${backend}.cpp")
      list(APPEND renderer_sources "${WEBENGINE_SOURCE_DIR}/src/${backend}.cpp")
    endif()
  endforeach()
  add_library(paws_engine_browser STATIC ${renderer_sources})
  add_library(WebEngine::Browser ALIAS paws_engine_browser)
  target_include_directories(paws_engine_browser PRIVATE "${CMAKE_BINARY_DIR}/engine/generated")
  target_link_libraries(paws_engine_browser PUBLIC engine_core engine_gui)
  # The pinned renderer's public header includes WebGPU types, so the port is
  # also needed while compiling consumers, not just while linking.
  target_compile_options(paws_engine_browser PUBLIC --use-port=emdawnwebgpu -fexceptions)
  target_compile_options(paws_engine_browser PRIVATE -Wall -Wextra -Wpedantic)
  target_link_options(paws_engine_browser INTERFACE --use-port=emdawnwebgpu -fexceptions
    -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2)
endfunction()
