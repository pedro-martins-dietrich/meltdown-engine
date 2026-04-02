#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinates;
layout(location = 2) in vec3 normal;

layout(location = 4) in mat4 modelMatrix;

layout(location = 0) out vec2 fragTextureCoordinates;
layout(location = 1) out vec3 fragNormal;

layout(set = 0, binding = 0) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
	mat4 projectionView;
} camera;

void main()
{
	fragTextureCoordinates = textureCoordinates;
	fragNormal = (modelMatrix * vec4(normal, 0.0f)).xyz;

	gl_Position = camera.projectionView * (modelMatrix * vec4(position, 1.0f));
}
