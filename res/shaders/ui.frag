#version 450

layout(binding = 0) uniform usampler2D texSampler;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(texCoord, 0, 1); // texture(texSampler, texCoord);
}
