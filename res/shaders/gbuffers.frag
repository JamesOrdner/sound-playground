#version 420 core

layout(location = 0) in vec3 wsCoords;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoord;
layout(location = 3) in vec4 shadowCoord;

layout (binding = 0) uniform sampler2DShadow shadowTex;

layout(location = 0) out uvec4 color0;
layout(location = 1) out vec4  color1;

void main()
{
	vec3 color = vec3(0.8);

	// efficient packing -- saved memory bandwidth offsets calculation cost
	color0.x = packHalf2x16(color.xy);
	color0.y = packHalf2x16(vec2(color.z, normal.x));
	color0.z = packHalf2x16(normal.yz);
	color1.xyz = wsCoords;
	color1.w = textureProj(shadowTex, shadowCoord);
}
