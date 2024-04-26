#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
	float colorIntensity = dot(fragNormal, vec3(0.0f, 0.0f, -1.0f));

	outColor = max(0.2f, colorIntensity * colorIntensity) * vec4(fragColor, 1.0f);
}
