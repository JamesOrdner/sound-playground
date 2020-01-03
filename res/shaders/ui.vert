#version 420 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform mat3 transform;

layout(location = 0) out vec2 outTexCoord;

void main()
{
	gl_Position = vec4((transform * vec3(position, 1)).xy, 0, 1);
	outTexCoord = texCoord;
}
