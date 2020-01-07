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
	// BEGIN unpack

	ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);

	vec2 temp = unpackHalf2x16(data0.y);
	vec3 gColor = vec3(unpackHalf2x16(data0.x), temp.x);
	vec3 gNormal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	vec3 gWSCoord = data1.xyz;
	vec4 shadowSelect = unpackSnorm4x8(floatBitsToUint(data1.w));
	float gShadow = shadowSelect.x;
	float gSelected = shadowSelect.y;

	// END unpack

	vec3 light = normalize(vec3(2, 3, -0.5));
	float ambient = gNormal.z * 0.1 + 0.1;
	float lit = max(dot(light, gNormal), 0) * gShadow + ambient;

	int spacing = 2;
	float highlightNeighbor = 0;
	for (int x = -1; x <= 1; x += 2) {
		for (int y = -1; y <= 1; y += 2) {
			float texel = texelFetch(gbuf1, ivec2(coord.x + spacing * x, coord.y + spacing * y), 0).w;
			highlightNeighbor += unpackSnorm4x8(floatBitsToUint(texel)).y;
		}
	}

	if (gSelected < 0.5 && highlightNeighbor > 0.5) {
		color = vec4(1, 0, 0, 1);
	}
	else {
		color = vec4(vec3(gColor * lit), 1);
	}
}
