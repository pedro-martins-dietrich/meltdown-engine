#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextureCoordinates;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 lightDirection = normalize(vec3(-0.6f, -0.8f, -1.0f));
	float lightIntensity = 0.8f * (dot(fragNormal, lightDirection) + 0.25f);

	outColor = max(0.2f, lightIntensity) * vec4(fragColor, 1.0f);
}
