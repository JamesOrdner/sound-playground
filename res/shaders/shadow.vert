#version 410 core

layout(location = 0) in vec3 position;
layout(location = 3) in mat4 instanceTransform;

uniform mat4 viewProj;

void main()
{
	gl_Position = viewProj * instanceTransform * vec4(position, 1.0);
}
