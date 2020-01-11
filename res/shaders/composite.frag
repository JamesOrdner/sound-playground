#version 460 core

layout(binding = 0) uniform usampler2D gbuf0;
layout(binding = 1) uniform sampler2D  gbuf1;

layout(location = 2) uniform mat4 viewMat;
layout(location = 3) uniform vec3 samplePoints[64];

layout(location = 0) out vec4 color;

void main()
{
	// BEGIN unpack
	//
	ivec2 coord = ivec2(gl_FragCoord.x, gl_FragCoord.y);
	uvec4 data0 = texelFetch(gbuf0, coord, 0);
	vec4 data1 = texelFetch(gbuf1, coord, 0);
	vec2 temp = unpackHalf2x16(data0.y);
	uint shadSelDepthPacked = floatBitsToUint(data1.w);
	vec4 shadowSelect = unpackUnorm4x8(shadSelDepthPacked & 0x0000FFFF);

	vec3 gColor = vec3(unpackHalf2x16(data0.x), temp.x);
	vec3 gNormal = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	vec3 gWSCoord = data1.xyz;
	float gShadow = shadowSelect.x;
	float gSelected = shadowSelect.y;
	float gDepth = unpackUnorm2x16(shadSelDepthPacked & 0xFFFF0000).y;
	//
	// END unpack

	// BEGIN ambient occlusion
	//
	float ao_radius = 40.0;
	float occ = 0.0;
	vec3 gNormalViewSpace = normalize(vec3(viewMat * vec4(gNormal, 0)) * 0.5 + 0.5);
	for (int i = 0; i < 64; i++) {
		vec3 dir = samplePoints[i];
		if (dot(gNormalViewSpace, dir) < 0.0) dir = -dir;
		
		for (int j = 1; j <= 4; j++) {
			float r = ao_radius * j / 4; // distance along dir
			vec4 dataOther = texelFetch(gbuf1, coord + ivec2(dir.xy * r), 0);
			float depthOther = unpackUnorm2x16(floatBitsToUint(dataOther.w) & 0xFFFF0000).y;
			
			float z = gDepth - dir.z * r * 0.00001;
			float dist = distance(gWSCoord, dataOther.xyz);
			if (z > depthOther && dist < 1) occ += 1 - dist;
		}
	}
	float ao = 1.0 - occ / (64 * 4);
	//
	// END ambient occlusion

	vec3 light = normalize(vec3(2, 3, -0.5));
	float ambient = gNormal.y * 0.25 + 0.25;
	vec3 lit = max(dot(light, gNormal), 0) * gShadow * vec3(1.0, 0.95, 0.9) + ambient * ao * vec3(.7, .9, 1) * 1.1;
	
	color = vec4(vec3(gColor * lit), 1);
	
	if (gSelected > 0.5) {
		color = mix(color, vec4(1.0, 0.6, 0.2, 1), 0.4);
	}
}
