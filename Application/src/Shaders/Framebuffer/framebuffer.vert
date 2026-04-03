#version 450

layout(location = 0) out vec2 fragCoordinates;

const vec2 vertices[3] =
{
	vec2(-1.0f, 1.0f),
	vec2(3.0f, 1.0f),
	vec2(-1.0f, -3.0f)
};

void main()
{
	fragCoordinates = 0.5f * (vertices[gl_VertexIndex] + 1.0f);

	gl_Position = vec4(vertices[gl_VertexIndex], 0.0f, 1.0f);
}
