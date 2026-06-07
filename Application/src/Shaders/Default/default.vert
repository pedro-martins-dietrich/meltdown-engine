#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinates;
layout(location = 2) in vec3 normal;

layout(location = 3) in mat4 transform;
layout(location = 7) in uvec4 instanceMaterialSet;

layout(location = 0) out vec2 fragTextureCoordinates;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) flat out uint materialSetID;

layout(set = 0, binding = 0) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
	mat4 projectionView;
} camera;

void main()
{
	fragTextureCoordinates = textureCoordinates;
	fragNormal = (transform * vec4(normal, 0.0f)).xyz;
	materialSetID = instanceMaterialSet.x;

	gl_Position = camera.projectionView * (transform * vec4(position, 1.0f));
}
