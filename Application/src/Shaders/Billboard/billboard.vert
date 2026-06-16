#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in mat4 transform;
layout(location = 5) in uvec4 instanceMaterialSet;

layout(location = 0) out vec2 fragUV;
layout(location = 1) flat out uint materialIdOffset;

layout(push_constant) uniform PushConstant
{
	uint materialSlot;
} pushConstant;

layout(set = 0, binding = 0) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
	mat4 projectionView;
} camera;

void main()
{
	vec4 worldPosition = camera.view * vec4(transform[3][0], transform[3][1], transform[3][2], 1.0f);
	vec4 vertexPosition = vec4(position.x * transform[0][0], position.y * transform[1][1], 0.0f, 0.0f);

	fragUV = 0.5f * (position + vec2(1.0f, 1.0f));
	materialIdOffset = instanceMaterialSet.x + pushConstant.materialSlot;

	gl_Position = camera.projection * (worldPosition + vertexPosition);
}
