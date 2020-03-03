# ---------- gltf

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gltf)

# ---------- nlohmann json

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann)

# ---------- PortAudio

if(WIN32)
  target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/include)
  target_link_libraries(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/lib/portaudio_x64.lib)
endif()

# ---------- Vulkan Memory Allocator

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vma)