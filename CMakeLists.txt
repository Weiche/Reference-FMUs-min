cmake_minimum_required (VERSION 3.17)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

FUNCTION(cat IN_FILE OUT_FILE)
  file(READ ${IN_FILE} CONTENTS)
  file(APPEND ${OUT_FILE} "${CONTENTS}")
ENDFUNCTION()

project (Reference-FMUs)

set(FMI_VERSION 2 CACHE STRING "FMI Version")
set_property(CACHE FMI_VERSION PROPERTY STRINGS 2 3) # Remove FMI1.0 Support

set(FMI_TYPE ME CACHE STRING "FMI Type (FMI 1.0 only)")
set_property(CACHE FMI_TYPE PROPERTY STRINGS ME CS)

if (${FMI_VERSION} GREATER 1)
  set(FMI_TYPE "")
endif ()

set(FMI_ARCHITECTURE "" CACHE STRING "FMI Architecture")
set_property(CACHE FMI_ARCHITECTURE PROPERTY STRINGS "" "aarch64" "x86" "x86_64")

if (NOT FMI_ARCHITECTURE)
  if (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "AMD64|x86_64")
    set(FMI_ARCHITECTURE "x86_64")
  elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set(FMI_ARCHITECTURE "aarch64")
  else ()
    message(FATAL_ERROR "Unknown System Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif ()
endif ()

set(WITH_FMUSIM OFF CACHE BOOL "Add fmusim project")

if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

  # Minimum supported MSVC version is 1800 = Visual Studio 12.0/2013
  # See also https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html
  if (MSVC_VERSION VERSION_LESS 1800)
    message (SEND_ERROR "MSVC_VERSION ${MSVC_VERSION} is not a supported Visual Studio compiler version. Please use Visual Studio 2013 or any later version.")
  endif ()
endif ()

# hide internal functions
if (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif()

if (${FMI_VERSION} GREATER 2)

  if (WIN32)
    set(FMI_PLATFORM "${FMI_ARCHITECTURE}-windows")
  elseif (APPLE)
    set(FMI_PLATFORM "${FMI_ARCHITECTURE}-darwin")
  else ()
    set(FMI_PLATFORM "${FMI_ARCHITECTURE}-linux")
  endif ()

else ()

  if (WIN32)
    set(FMI_PLATFORM win)
  elseif (APPLE)
    set(FMI_PLATFORM darwin)
  else ()
    set(FMI_PLATFORM linux)
  endif ()

  if ("${FMI_ARCHITECTURE}" STREQUAL "x86_64")
    set (FMI_PLATFORM ${FMI_PLATFORM}64)
  elseif ("${FMI_ARCHITECTURE}" STREQUAL "x86")
    set (FMI_PLATFORM ${FMI_PLATFORM}32)
  else ()
    message(FATAL_ERROR "Unsupported architecture for FMI version < 3.0: ${FMI_ARCHITECTURE}")
  endif ()

endif ()

if (APPLE)

  if ("${FMI_ARCHITECTURE}" STREQUAL "x86_64")
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
  elseif ("${FMI_ARCHITECTURE}" STREQUAL "aarch64")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
  else ()
    message(FATAL_ERROR "Unsupported architecture darwin: ${FMI_ARCHITECTURE}")
  endif ()

endif ()

message("FMI_PLATFORM: " ${FMI_PLATFORM})

if (${FMI_VERSION} LESS 3)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/all.c" "#define FMI_VERSION ${FMI_VERSION}

#include \"fmi${FMI_VERSION}Functions.c\"
#include \"model.c\"
#include \"cosimulation.c\"
")
endif ()
#
# Here defines what FMU will be included in the configure
set (EXAMPLE_MODEL_NAMES  BouncingBall Dahlquist Stair Feedthrough VanDerPol)

if (${FMI_VERSION} GREATER 2)
set (EXAMPLE_MODEL_NAMES ${EXAMPLE_MODEL_NAMES} StateSpace Clocks)
endif ()


if (IGNORE_EXAMPLES)
set (MODEL_NAMES ${EXT_MODEL_NAMES})
else()
set (MODEL_NAMES ${EXT_MODEL_NAMES} ${EXAMPLE_MODEL_NAMES})
endif()

foreach (MODEL_NAME ${MODEL_NAMES})

set(TARGET_NAME ${MODEL_NAME})
set(MODEL_PATH fmu_examples/${MODEL_NAME})

SET(HEADERS
    ${MODEL_PATH}/config.h
    include/cosimulation.h
    include/model.h
)

if (${FMI_VERSION} EQUAL 2)
  SET(HEADERS
    ${HEADERS}
    include/fmi2Functions.h
    include/fmi2FunctionTypes.h
    include/fmi2TypesPlatform.h
  )
elseif (${FMI_VERSION} EQUAL 3)
  SET(HEADERS
    ${HEADERS}
    include/fmi3Functions.h
    include/fmi3FunctionTypes.h
    include/fmi3PlatformTypes.h
  )
endif()

SET(SOURCES
  ${MODEL_PATH}/model.c
  src/fmi${FMI_VERSION}Functions.c
  src/cosimulation.c
)

add_library(${TARGET_NAME} SHARED
  ${HEADERS}
  ${SOURCES}
  ${MODEL_PATH}/FMI${FMI_VERSION}${FMI_TYPE}.xml
  ${MODEL_PATH}/buildDescription.xml
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fmus)

target_compile_definitions(${TARGET_NAME} PRIVATE
  FMI_VERSION=${FMI_VERSION}
  DISABLE_PREFIX
)

if (MSVC)
  target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  target_link_options(${TARGET_NAME} PRIVATE "-static-libgcc")
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Intel")
  target_link_options(${TARGET_NAME} PRIVATE "-static-intel" "-static-libgcc")
endif()

if (${FMI_VERSION} EQUAL 1 AND "${FMI_TYPE}" STREQUAL CS)
  target_compile_definitions(${TARGET_NAME} PRIVATE FMI_COSIMULATION)
endif()

target_include_directories(${TARGET_NAME} PRIVATE
  include
  ${MODEL_PATH}
)

set(FMU_BUILD_DIR temp/${MODEL_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY         "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY         "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY         "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG   "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}"
)

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${MODEL_NAME})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# modelDescription.xml
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_PATH}/FMI${FMI_VERSION}${FMI_TYPE}.xml
  "${FMU_BUILD_DIR}/modelDescription.xml"
)

# model specific header and source
foreach (SOURCE_FILE config.h model.c)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_PATH}/${SOURCE_FILE}"
    "${FMU_BUILD_DIR}/sources/${SOURCE_FILE}"
  )
endforeach(SOURCE_FILE)

# common headers
foreach (SOURCE_FILE model.h cosimulation.h)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/include/${SOURCE_FILE}"
    "${FMU_BUILD_DIR}/sources/${SOURCE_FILE}"
  )
endforeach(SOURCE_FILE)

# common sources
foreach (SOURCE_FILE fmi${FMI_VERSION}Functions.c cosimulation.c)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${SOURCE_FILE}"
    "${FMU_BUILD_DIR}/sources/${SOURCE_FILE}"
  )
endforeach(SOURCE_FILE)

# all.c / buildDescription.xml
if (${FMI_VERSION} LESS 3)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/all.c"
    "${FMU_BUILD_DIR}/sources/all.c"
  )
else()
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_PATH}/buildDescription.xml"
    "${FMU_BUILD_DIR}/sources/buildDescription.xml"
  )
endif()

set(ARCHIVE_FILES "modelDescription.xml" "binaries" "sources")

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_PATH}/resources")
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_PATH}/resources"
    "${FMU_BUILD_DIR}/resources/"
  )
  set(ARCHIVE_FILES ${ARCHIVE_FILES} "resources")
endif()

# delete unintended files from binaries (Release configuration only)
if (MSVC)
  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND if $<CONFIG:Release> neq 0 (
      ${CMAKE_COMMAND} -E rm -f
      "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}/${MODEL_NAME}.exp"
      "${FMU_BUILD_DIR}/binaries/${FMI_PLATFORM}/${MODEL_NAME}.lib"
     )
  )
endif()

# create ZIP archive
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${CMAKE_CURRENT_BINARY_DIR}/fmus/${MODEL_NAME}.fmu --format=zip
  ${ARCHIVE_FILES}
  WORKING_DIRECTORY ${FMU_BUILD_DIR} COMMENT "Creating ZIP archive"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fmus/${MODEL_NAME}.fmu DESTINATION ${CMAKE_INSTALL_PREFIX})

endforeach(MODEL_NAME)
