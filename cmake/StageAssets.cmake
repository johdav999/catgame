# Only this explicit runtime tree is deployable. Masters stay under artifacts/.
file(GLOB_RECURSE RUNTIME_FILES RELATIVE "${SOURCE_DIR}/assets/runtime" "${SOURCE_DIR}/assets/runtime/*")
foreach(ASSET IN LISTS RUNTIME_FILES)
  if(ASSET MATCHES "\\.(blend|psd|kra|py|bak)$" OR ASSET MATCHES "(^|/)(sources|raw|rejected)/")
    message(FATAL_ERROR "Source master in runtime tree: ${ASSET}")
  endif()
endforeach()
file(COPY "${SOURCE_DIR}/assets/runtime/" DESTINATION "${SITE_DIR}/assets")
