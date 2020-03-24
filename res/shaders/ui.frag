#version 450

// layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 color;

void main()
{
	// color = texture(texSampler, texCoord);
	color = vec4(texCoord, 0, 1);
}
