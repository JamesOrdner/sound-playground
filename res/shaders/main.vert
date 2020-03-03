#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform Transform {
    mat4 matrix;
} transform;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragNormal;

void main()
{
    gl_Position = transform.matrix * vec4(position, 1.0);
	fragNormal = vec3(transform.matrix * vec4(normal, 0)) * 0.5 + 0.5;
}
