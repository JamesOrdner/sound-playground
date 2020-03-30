#version 450

layout(set = 0, binding = 0) uniform sampler2D shadowMap;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 lightDir;
layout(location = 2) in vec3 shadowCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 directLight = max(dot(normal, lightDir), 0) * vec3(1, .94, .94);
	vec3 ambientLight = vec3(0.25, 0.4, 0.45) * 0.4;
	float shadowDepth = texture(shadowMap, vec2(shadowCoord.x, shadowCoord.y)).r;
	float shadow = shadowDepth < shadowCoord.z - 0.001 ? 0 : 1;
	outColor = vec4(directLight * shadow + ambientLight, 1.0);
}
