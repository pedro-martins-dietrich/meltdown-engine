#version 450

layout(location = 0) out vec3 fragColor;

// Triangle vertices
vec3 positions[3] =
{
	vec3(0.0f, -0.8f, 0.5f),
	vec3(-0.7f, 0.7f, 0.5f),
	vec3(0.7f, 0.7f, 0.5f)
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

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}
