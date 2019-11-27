#version 420 core

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 texCoord;
layout(location = 2) in vec4 shadowCoord;

layout (binding = 0) uniform sampler2DShadow shadow_tex;

layout(location = 0) out vec4 color;

void main() {
	vec3 light = normalize(vec3(2, 3, -0.5));

	float ambient = normal.z * 0.1 + 0.1;

	float val = max(dot(light, normal), 0);
	vec4 lit = vec4(val, val, val, 1);
	
	color = textureProj(shadow_tex, shadowCoord) * lit + ambient;
}
