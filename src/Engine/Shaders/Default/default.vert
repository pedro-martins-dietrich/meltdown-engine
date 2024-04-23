#version 450

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform CameraMatrices
{
	mat4 projection;
	mat4 view;
} camera;

// Triangle vertices
vec3 positions[3] =
{
	vec3(0.0f, -2.0f, 0.0f),
	vec3(-1.732f, 1.0f, 0.0f),
	vec3(1.732f, 1.0f, 0.0f)
};

// Triangle colors
vec3 colors[3] =
{
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f)
};

// Renders a triangle to screen
void main()
{
	fragColor = colors[gl_VertexIndex];

	gl_Position = camera.projection * (camera.view * vec4(positions[gl_VertexIndex], 1.0f));
}
