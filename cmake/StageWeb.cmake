# Explicit production shell. Inspection shells and executables are separate targets.
foreach(NAME index.html host.js saves.js game-platform.js cloud.js paws-audio.js style.css favicon.svg)
  configure_file("${SOURCE_DIR}/web/${NAME}" "${SITE_DIR}/${NAME}" COPYONLY)
endforeach()
