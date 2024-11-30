#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextureCoordinates;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D diffuse;

layout(set = 2, binding = 0) uniform LightData
{
	vec3 lightDirection;
	float ambientLightIntensity;
} lightData;

void main()
{
	float lightIntensity = 0.8f * (dot(fragNormal, -lightData.lightDirection) + 0.25f);

	outColor = max(lightData.ambientLightIntensity, lightIntensity) * texture(diffuse, fragTextureCoordinates);
}
