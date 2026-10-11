file(MAKE_DIRECTORY "${SITE_DIR}/licenses")
foreach(entry "ufbx/LICENSE" "cgltf/LICENSE" "stb/LICENSE" "meshoptimizer/LICENSE.md")
  string(REGEX REPLACE "/.*" "" dependency "${entry}")
  configure_file("${ENGINE_SOURCE_DIR}/third_party/${entry}"
    "${SITE_DIR}/licenses/${dependency}.txt" COPYONLY)
endforeach()
configure_file("${JOLT_SOURCE_DIR}/LICENSE"
  "${SITE_DIR}/licenses/Jolt.txt" COPYONLY)
foreach(dependency Emscripten Dawn)
  configure_file("${SOURCE_DIR}/licenses/${dependency}.txt"
    "${SITE_DIR}/licenses/${dependency}.txt" COPYONLY)
endforeach()
