#version 450

layout(location = 0) in vec2 fragCoordinates;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D inputFramebufferColor;

void main()
{
	outColor = texture(inputFramebufferColor, fragCoordinates);
}
