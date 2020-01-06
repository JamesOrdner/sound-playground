#version 420 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

uniform mat3 transform;
uniform vec4 uiTexCoord; // { x0, y0, x1, y1 }

layout(location = 0) out vec2 outTexCoord;

void main()
{
	gl_Position = vec4((transform * vec3(position, 1)).xy, 0, 1);
	float texCoordX = (uiTexCoord[2] - uiTexCoord[0]) * texCoord.x + uiTexCoord[0];
	float texCoordY = (uiTexCoord[3] - uiTexCoord[1]) * texCoord.y + uiTexCoord[1];
	outTexCoord = vec2(texCoordX, texCoordY);
}
