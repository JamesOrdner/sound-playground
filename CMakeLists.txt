cmake_minimum_required(VERSION 3.16)

project(SoundPlayground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(XCODE)
  set(CMAKE_XCODE_GENERATE_SCHEME ON)
  set(CMAKE_XCODE_SCHEME_ENVIRONMENT ON)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_executable(SoundPlayground)

add_subdirectory(src)

get_property("TARGET_SOURCE_FILES" TARGET SoundPlayground PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" FILES ${TARGET_SOURCE_FILES})

# ---------- EXTERNAL DEPENDENCIES ----------

add_subdirectory(lib)

include(ExternalProject)
set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)
set(DEPENDENCY_CMAKE_ARGS
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DCMAKE_BUILD_TYPE=Release
)

# ---------- FFTW

set(FFTW_CACHE_ARGS
  -DENABLE_FLOAT:BOOL=TRUE
  -DBUILD_TESTS:BOOL=FALSE
)
ExternalProject_Add(FFTW
  URL http://www.fftw.org/fftw-3.3.8.tar.gz
  PREFIX ${DEPENDENCIES_DIR}
  CMAKE_ARGS ${DEPENDENCY_CMAKE_ARGS}
  CMAKE_CACHE_ARGS ${FFTW_CACHE_ARGS}
  INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release
)
target_include_directories(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/include)
if (WIN32)
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/fftw3f.lib)
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DEPENDENCIES_DIR}/bin/fftw3f.dll"
    $<TARGET_FILE_DIR:SoundPlayground>
  )
elseif(APPLE)
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/libfftw3f.dylib)
endif()

# ---------- PortAudio

if (WIN32)
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/portaudio/lib/portaudio_x64.dll"
    $<TARGET_FILE_DIR:SoundPlayground>
  )
elseif (APPLE)
  ExternalProject_Add(PortAudio
    URL http://www.portaudio.com/archives/pa_stable_v190600_20161030.tgz
    PREFIX ${DEPENDENCIES_DIR}
    CMAKE_ARGS ${DEPENDENCY_CMAKE_ARGS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --target install --config Release
  )
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/libportaudio.dylib)
endif()

# ---------- SDL2

if (WIN32) # use precompiled binaries
  ExternalProject_Add(SDL2
    URL https://www.libsdl.org/release/SDL2-devel-2.0.10-VC.zip
    PREFIX ${DEPENDENCIES_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCIES_DIR}/src/SDL2/lib/x64 ${DEPENDENCIES_DIR}/lib
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${DEPENDENCIES_DIR}/src/SDL2/include ${DEPENDENCIES_DIR}/include/SDL2
  )
  target_include_directories(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/include/SDL2)
  target_link_libraries(SoundPlayground PRIVATE ${DEPENDENCIES_DIR}/lib/SDL2.lib ${DEPENDENCIES_DIR}/lib/SDL2main.lib)
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DEPENDENCIES_DIR}/lib/SDL2.dll"
    $<TARGET_FILE_DIR:SoundPlayground>
  )
else()
  find_package(SDL2 REQUIRED)
  target_include_directories(SoundPlayground PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(SoundPlayground PRIVATE ${SDL2_LIBRARIES})
endif()

# ---------- Vulkan

set(VULKAN_SDK_DIR "" CACHE STRING "Top level directory of the Vulkan SDK")

if (WIN32)
  target_include_directories(SoundPlayground PRIVATE ${VULKAN_SDK_DIR}/Include)
  target_link_libraries(SoundPlayground PRIVATE ${VULKAN_SDK_DIR}/Lib/vulkan-1.lib)
elseif(XCODE)
  target_include_directories(SoundPlayground PRIVATE ${VULKAN_SDK_DIR}/macOS/include)
  target_link_libraries(SoundPlayground
    PRIVATE
      ${VULKAN_SDK_DIR}/macOS/lib/libvulkan.1.dylib
      ${VULKAN_SDK_DIR}/macOS/lib/libvulkan.1.2.131.dylib
  )
  set(VK_ICD_FILENAMES_ENV VK_ICD_FILENAMES=${VULKAN_SDK_DIR}/macOS/etc/vulkan/icd.d/MoltenVK_icd.json)
  set(VK_LAYER_PATH_ENV VK_LAYER_PATH=${VULKAN_SDK_DIR}/macOS/etc/vulkan/explicit_layer.d)
  set_target_properties(SoundPlayground PROPERTIES XCODE_SCHEME_ENVIRONMENT "${VK_ICD_FILENAMES_ENV};${VK_LAYER_PATH_ENV}")
elseif(UNIX)
  include_directories(SoundPlayground PRIVATE ${VULKAN_SDK_DIR}/x86_64/include)
  target_link_libraries(SoundPlayground PRIVATE ${VULKAN_SDK_DIR}/x86_64/lib/libvulkan.so)
endif()

# ---------- GLSL to SPIR-V

find_program(GLSLC glslc
  HINTS ${VULKAN_SDK_DIR}/macOS/bin/ ${VULKAN_SDK_DIR}/Bin
)

set(SHADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/main.vert
  ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/main.frag
  ${CMAKE_CURRENT_SOURCE_DIR}/res/shaders/shadow.vert
)

foreach(SHADER ${SHADERS})
  add_custom_command(
    OUTPUT ${SHADER}.spv
    COMMAND ${GLSLC} -o ${SHADER}.spv ${SHADER}
    DEPENDS ${SHADER}
    VERBATIM
  )
  target_sources(SoundPlayground PRIVATE ${SHADER}.spv)
endforeach()

# ---------- COPY RESOURCES ----------

if(WIN32)
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    ${CMAKE_BINARY_DIR}/res
  )
else()
  add_custom_command(TARGET SoundPlayground POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:SoundPlayground>/res
  )
endif()
