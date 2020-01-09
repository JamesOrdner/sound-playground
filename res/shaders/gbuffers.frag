#version 420 core

layout(location = 0) in vec3 wsCoords;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoord;
layout(location = 3) in vec4 shadowCoord;
layout(location = 4) in float selected;

layout(binding = 0) uniform sampler2DShadow shadowTex;

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
	vec4 shadowSelect = vec4(textureProj(shadowTex, shadowCoord), selected, 0, 0);
	uint shadowSelectPacked = packUnorm4x8(shadowSelect);
	float f = 15.0; // far plane
	float n = 0.05; // near plane
	float linDepth = n * f / (f + gl_FragCoord.z * (n - f)) * 0.07;
	uint depthPacked = packUnorm2x16(vec2(0, linDepth));
	color1.w = uintBitsToFloat(shadowSelectPacked | depthPacked);
}
