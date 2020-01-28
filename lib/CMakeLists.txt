# ---------- PortAudio

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/include)
target_link_libraries(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/portaudio/lib/portaudio_x64.lib)

# ---------- TinyGLTF

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)

# ---------- GL3W

target_include_directories(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/include)
target_sources(SoundPlayground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/gl3w/src/gl3w.c)