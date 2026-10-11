cmake_minimum_required(VERSION 3.22)
project(Paws VERSION 0.1.0 LANGUAGES C CXX)
option(PAWS_GUI_GALLERY "Include development-only component gallery" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(WEBENGINE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/WebEngine" CACHE PATH
    "WebEngine checkout; shared builds use the pinned submodule")
get_filename_component(WEBENGINE_SOURCE_DIR "${WEBENGINE_SOURCE_DIR}" ABSOLUTE)
if(NOT EXISTS "${WEBENGINE_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "WebEngine is missing. Run git submodule update --init --recursive, or set WEBENGINE_SOURCE_DIR.")
endif()
file(READ "${WEBENGINE_SOURCE_DIR}/.emscripten-version" ENGINE_EMSCRIPTEN_VERSION)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.emscripten-version" PAWS_EMSCRIPTEN_VERSION)
string(STRIP "${ENGINE_EMSCRIPTEN_VERSION}" ENGINE_EMSCRIPTEN_VERSION)
string(STRIP "${PAWS_EMSCRIPTEN_VERSION}" PAWS_EMSCRIPTEN_VERSION)
if(EMSCRIPTEN AND NOT ENGINE_EMSCRIPTEN_VERSION STREQUAL PAWS_EMSCRIPTEN_VERSION)
  message(FATAL_ERROR "Paws requires Emscripten ${PAWS_EMSCRIPTEN_VERSION}; this engine checkout requires ${ENGINE_EMSCRIPTEN_VERSION}.")
endif()
message(STATUS "Paws WebEngine source: ${WEBENGINE_SOURCE_DIR}")
include(cmake/WebEngine.cmake)
paws_add_webengine()

add_library(paws_rules src/puzzle.cpp)
target_include_directories(paws_rules PUBLIC src)
add_library(paws_content src/content.cpp src/afternoon.cpp)
target_link_libraries(paws_content PUBLIC paws_rules)
add_library(paws_game src/starter_scene.cpp src/puzzle_scene.cpp src/garden_scene.cpp)
target_include_directories(paws_game PUBLIC src)
target_link_libraries(paws_game PUBLIC WebEngine::Core paws_content)
add_library(paws_ui src/paws_ui.cpp)
add_library(paws_audio src/game_audio.cpp)
target_include_directories(paws_audio PUBLIC src)
target_link_libraries(paws_audio PUBLIC WebEngine::Core paws_content)
target_include_directories(paws_ui PUBLIC src)
target_link_libraries(paws_ui PUBLIC engine_gui)
if(MSVC)
  target_compile_options(paws_game PRIVATE /W4)
else()
  target_compile_options(paws_game PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
  target_compile_options(paws_rules PRIVATE -fexceptions)
  target_compile_options(paws_content PRIVATE -fexceptions)
  target_compile_options(paws_game PRIVATE -fexceptions)
  target_compile_options(paws_audio PRIVATE -fexceptions)
  add_executable(paws src/browser_main.cpp)
  target_link_libraries(paws PRIVATE paws_game paws_ui paws_audio WebEngine::Browser engine_gui_browser engine_audio_browser)
  target_compile_definitions(paws PRIVATE PAWS_GUI_GALLERY=$<BOOL:${PAWS_GUI_GALLERY}>)
  target_link_options(paws PRIVATE
    -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=1048576 -sENVIRONMENT=web
    -sEXIT_RUNTIME=0 -sFILESYSTEM=0
    "-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']"
    "-sEXPORTED_RUNTIME_METHODS=['HEAPU8','UTF8ToString']"
    "$<$<CONFIG:Debug>:-sASSERTIONS=2>")
  set_target_properties(paws PROPERTIES SUFFIX ".js"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/site")
  # Each configuration gets its own deployable site; no writes into the engine.
  add_custom_target(paws_site ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/web" "${CMAKE_BINARY_DIR}/site"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_SOURCE_DIR}/web/gui.js" "${CMAKE_BINARY_DIR}/site/engine-gui.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_SOURCE_DIR}/web/storage.js" "${CMAKE_BINARY_DIR}/site/engine-storage.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${WEBENGINE_SOURCE_DIR}/web/audio-system.js" "${CMAKE_BINARY_DIR}/site/engine-audio.js"
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${PROJECT_SOURCE_DIR}
      -DSITE_DIR=${CMAKE_BINARY_DIR}/site -P "${PROJECT_SOURCE_DIR}/cmake/StageAssets.cmake"
    COMMAND ${CMAKE_COMMAND} -DENGINE_SOURCE_DIR=${WEBENGINE_SOURCE_DIR}
      -DJOLT_SOURCE_DIR=${PAWS_JOLT_SOURCE_DIR} -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -DSITE_DIR=${CMAKE_BINARY_DIR}/site
      -P "${PROJECT_SOURCE_DIR}/cmake/CopyLicenses.cmake"
    COMMAND ${CMAKE_COMMAND} -DENGINE_SOURCE_DIR=${WEBENGINE_SOURCE_DIR}
      -DSITE_DIR=${CMAKE_BINARY_DIR}/site -DVERSION=${PROJECT_VERSION}
      -DCONFIGURATION=$<CONFIG> -DGALLERY=$<IF:$<BOOL:${PAWS_GUI_GALLERY}>,true,false>
      -P "${PROJECT_SOURCE_DIR}/cmake/BuildInfo.cmake"
    DEPENDS paws
    COMMENT "Preparing the Paws browser site"
    VERBATIM)
else()
  include(CTest)
  if(BUILD_TESTING)
    add_executable(paws_audio_tests tests/audio_tests.cpp)
    target_link_libraries(paws_audio_tests PRIVATE paws_audio)
    add_test(NAME paws_audio COMMAND paws_audio_tests)
    add_executable(paws_scene_tests tests/starter_scene_tests.cpp)
    target_link_libraries(paws_scene_tests PRIVATE paws_game)
    add_test(NAME paws_starter_scene COMMAND paws_scene_tests)
    add_executable(paws_rules_tests tests/puzzle_tests.cpp)
    target_link_libraries(paws_rules_tests PRIVATE paws_content)
    add_test(NAME paws_rules COMMAND paws_rules_tests)
    add_executable(paws_ui_tests tests/paws_ui_tests.cpp)
    target_link_libraries(paws_ui_tests PRIVATE paws_ui)
    target_compile_definitions(paws_ui_tests PRIVATE PAWS_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME paws_ui COMMAND paws_ui_tests)
    add_executable(paws_phase6_scene_tests tests/phase6_scene_tests.cpp)
    target_link_libraries(paws_phase6_scene_tests PRIVATE paws_game)
    target_compile_definitions(paws_phase6_scene_tests PRIVATE PAWS_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME paws_phase6_scene COMMAND paws_phase6_scene_tests)
    add_executable(paws_phase5_scene_tests tests/phase5_scene_tests.cpp)
    target_link_libraries(paws_phase5_scene_tests PRIVATE paws_game)
    target_compile_definitions(paws_phase5_scene_tests PRIVATE PAWS_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME paws_phase5_scene COMMAND paws_phase5_scene_tests)
    add_executable(paws_phase7_tests tests/phase7_tests.cpp)
    target_link_libraries(paws_phase7_tests PRIVATE paws_content)
    add_test(NAME paws_phase7_rules COMMAND paws_phase7_tests)
    add_executable(paws_phase8_tests tests/phase8_tests.cpp)
    target_link_libraries(paws_phase8_tests PRIVATE paws_content)
    add_test(NAME paws_phase8_saves COMMAND paws_phase8_tests)
    add_executable(paws_phase7_scene_tests tests/phase7_scene_tests.cpp)
    target_link_libraries(paws_phase7_scene_tests PRIVATE paws_game)
    target_compile_definitions(paws_phase7_scene_tests PRIVATE PAWS_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
    add_test(NAME paws_phase7_scene COMMAND paws_phase7_scene_tests)
  endif()
endif()

include(cmake/PipPreview.cmake)
include(cmake/GardenPreview.cmake)
target_link_libraries(paws_game PUBLIC paws_garden)
if(EMSCRIPTEN)
  target_link_libraries(paws PRIVATE engine_assets_browser)
endif()
