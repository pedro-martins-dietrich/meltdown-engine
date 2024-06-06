#version 450

layout(location = 0) in mat4 model;

layout(location = 0) out vec2 fragTextureCoordinates;

layout(set = 0, binding = 0) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
	mat4 projectionView;
} camera;

const vec2 vertices[6] =
{
	vec2(-1.0f, -1.0f),
	vec2(1.0f, -1.0f),
	vec2(1.0f, 1.0f),
	vec2(-1.0f, -1.0f),
	vec2(-1.0f, 1.0f),
	vec2(1.0f, 1.0f)
};

void main()
{
	vec4 worldPosition = camera.view * vec4(model[3][0], model[3][1], model[3][2], 1.0f);
	vec4 vertexPosition = vec4
	(
		vertices[gl_VertexIndex].x * model[0][0],
		vertices[gl_VertexIndex].y * model[1][1],
		0.0f, 0.0f
	);

	fragTextureCoordinates = 0.5f * (vertices[gl_VertexIndex] + 1.0f);

	gl_Position = camera.projection * (worldPosition + vertexPosition);
}
