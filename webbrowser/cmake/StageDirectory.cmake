# Utility script invoked at build time to copy directories as part of the
# post-build stage.  The caller must define the following variables:
#   SRC_DIR  - source directory to copy.
#   DST_DIR  - destination directory that will receive the data.
# Optional variables:
#   COPY_CONTENTS - if truthy, copy the contents of SRC_DIR into DST_DIR.

if(NOT DEFINED SRC_DIR)
    message(FATAL_ERROR "StageDirectory: SRC_DIR was not provided")
endif()

if(NOT DEFINED DST_DIR)
    message(FATAL_ERROR "StageDirectory: DST_DIR was not provided")
endif()

if(NOT EXISTS "${SRC_DIR}")
    message(FATAL_ERROR "StageDirectory: source directory '${SRC_DIR}' does not exist")
endif()

file(MAKE_DIRECTORY "${DST_DIR}")

# When COPY_CONTENTS is enabled copy the children of SRC_DIR into DST_DIR.
if(COPY_CONTENTS)
    file(GLOB _stage_children RELATIVE "${SRC_DIR}" "${SRC_DIR}/*")
    foreach(_child IN LISTS _stage_children)
        file(COPY "${SRC_DIR}/${_child}" DESTINATION "${DST_DIR}")
    endforeach()
else()
    file(COPY "${SRC_DIR}" DESTINATION "${DST_DIR}")
endif()
