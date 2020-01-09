#version 420 core

layout(location = 0) in  vec3 position;
layout(location = 1) in  vec3 normal;
layout(location = 2) in  vec3 texCoord;
layout(location = 3) in  mat4 instanceTransform;
layout(location = 7) in  float selected;

layout(location = 0) out vec3 wsCoords;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTexCoord;
layout(location = 3) out vec4 shadowCoord;
layout(location = 4) out float outSelected;

uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat4 shadowViewProj;

void main()
{
	wsCoords = vec3(instanceTransform * vec4(position, 1.0));
	outNormal = vec3(instanceTransform * vec4(normal, 0));
	outTexCoord = texCoord;
	shadowCoord = shadowViewProj * vec4(wsCoords, 1.0);
	outSelected = selected;

	gl_Position = projMat * viewMat * vec4(wsCoords, 1.0);
}
