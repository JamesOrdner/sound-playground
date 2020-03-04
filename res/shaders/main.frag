#version 450

layout(location = 0) in vec3 normal;

layout(location = 0) out vec4 outColor;

void main()
{
	float light = dot(normalize(vec3(.3, 1, .1)), normal);
    outColor = vec4(vec3(.9) * light, 1.0);
}
