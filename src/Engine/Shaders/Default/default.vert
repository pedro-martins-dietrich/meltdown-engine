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

layout(set = 0, binding = 0, std140) readonly buffer storageBuffer
{
	mat4 model[];
} modelData;

void main()
{
	mat4 model = modelData.model[gl_InstanceIndex];

	fragColor = color;
	fragNormal = (model * vec4(normal, 0.0f)).xyz;

	gl_Position = camera.projection * (camera.view * (model * vec4(position, 1.0f)));
}
