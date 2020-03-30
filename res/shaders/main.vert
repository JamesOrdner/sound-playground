#version 450

layout(push_constant) uniform Constants {
    mat4 projectionMatrix;
};

layout(set = 1, binding = 0) uniform Transforms {
    mat4 modelViewMatrix;
	mat4 shadowMatrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 lightDir;
layout(location = 2) out vec3 shadowCoord;

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
	fragNormal = vec3(modelViewMatrix * vec4(normal, 0));
	lightDir = vec3(modelViewMatrix * vec4(normalize(vec3(.3, 1, .1)), 0));
	shadowCoord = vec3(shadowMatrix * vec4(position, 1.0));
	shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
	shadowCoord.y = shadowCoord.y * 0.5 + 0.5;
}
