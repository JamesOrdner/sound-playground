#version 420 core

layout(location = 0) in  vec3 position;
layout(location = 1) in  vec3 normal;
layout(location = 2) in  vec3 texCoord;
layout(location = 3) in  mat4 instanceTransform;

layout(location = 0) out vec3 wsCoords;
layout(location = 1) out vec3 outNormal;
// layout(location = 2) out vec3 outTexCoord;

uniform mat4 viewProj;

void main()
{
	wsCoords = vec3(instanceTransform * vec4(position, 1.0));
	outNormal = normal;
	// outTexCoord = texCoord;

	gl_Position = viewProj * vec4(wsCoords, 1.0);
}
