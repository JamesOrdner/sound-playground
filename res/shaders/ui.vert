#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
	gl_Position = vec4(position, 0, 1);
	outTexCoord = texCoord;
}
