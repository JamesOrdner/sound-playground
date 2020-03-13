#version 450

layout(binding = 0) uniform UBO {
    mat4 mvpMatrix;
};

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = mvpMatrix * vec4(position, 1.0);
}
