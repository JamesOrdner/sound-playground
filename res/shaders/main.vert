#version 410 core

layout(location = 0) in  vec3 position;
layout(location = 1) in  vec3 normal;
layout(location = 2) in  vec3 texCoord;
layout(location = 3) in  mat4 instanceTransform;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outTexCoord;
layout(location = 2) out vec4 shadow_coord;

uniform mat4 viewProj;
uniform mat4 shadowMVP;

void main() {
	gl_Position = viewProj * instanceTransform * vec4(position, 1.0);
	outNormal = normal;
	outTexCoord = texCoord;
	shadow_coord = shadowMVP * instanceTransform * vec4(position, 1.0);
}
