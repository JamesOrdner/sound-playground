#version 450

layout(binding = 2) uniform sampler2D shadowMap;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 lightDir;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 directLight = max(dot(normal, lightDir), 0) * vec3(1, .94, .94);
	vec3 ambientLight = vec3(0.25, 0.4, 0.45) * 0.4;
	outColor = vec4(directLight + ambientLight, 1.0);
}
