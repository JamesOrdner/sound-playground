#version 420 core

layout (binding = 0) uniform usampler2D gbuf0;
layout (binding = 1) uniform sampler2D  gbuf1;

layout(location = 0) out vec4 color;

float specularPower(ivec2 coord)
{
	return texelFetch(gbuf1, coord, 0).w;
}

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);

	vec2 temp = unpackHalf2x16(data0.y);
	vec3 gColor = vec3(unpackHalf2x16(data0.x), temp.x);
	vec3 gNormal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	vec3 gWSCoord = data1.xyz;
	float gShadow = data1.w;

	vec3 light = normalize(vec3(2, 3, -0.5));
	float ambient = gNormal.z * 0.1 + 0.1;
	float lit = max(dot(light, gNormal), 0) * gShadow + ambient;

//	int spacing = 2;
//	float highlightNeighbor = texelFetch(gbuf1, ivec2(coord.x + spacing, coord.y + spacing), 0).w;
//	highlightNeighbor += texelFetch(gbuf1, ivec2(coord.x + spacing, coord.y - spacing), 0).w;
//	highlightNeighbor += texelFetch(gbuf1, ivec2(coord.x - spacing, coord.y + spacing), 0).w;
//	highlightNeighbor += texelFetch(gbuf1, ivec2(coord.x - spacing, coord.y - spacing), 0).w;

//	if (gSpecularPower < 1 && highlightNeighbor > 1) {
//		color = vec4(1, 0, 0, 1);
//	}
//	else {
//		color = vec4(vec3(lit + ambient), 1);
//	}
	
	color = vec4(vec3(gColor * lit), 1);
}
