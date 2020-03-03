target_sources(SoundPlayground
  PRIVATE
    CameraGraphicsObject.cpp
    CameraGraphicsObject.h
    GraphicsObject.cpp
    GraphicsObject.h
    GraphicsScene.cpp
    GraphicsScene.h
    GraphicsSystem.cpp
    GraphicsSystem.h
    GraphicsSystemInterface.h
    MeshGraphicsObject.cpp
    MeshGraphicsObject.h
    UIGraphicsObject.cpp
    UIGraphicsObject.h
)

add_subdirectory(Vulkan)
