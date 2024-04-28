#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;

layout(push_constant) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
} camera;

void main()
{
	fragColor = color;
	fragNormal = normal;

	gl_Position = camera.projection * (camera.view * vec4(position, 1.0f));
}
