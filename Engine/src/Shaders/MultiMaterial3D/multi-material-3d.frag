#version 450

layout(location = 0) in vec2 fragTextureCoordinates;
layout(location = 1) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D diffuse;

void main()
{
	vec4 diffuseColor = texture(diffuse, fragTextureCoordinates);
	if(diffuseColor.a <= 0.05)
		discard;

	float lightIntensity = 0.8f * (dot(fragNormal, vec3(0.0f, -0.8f, -0.6f)) + 0.25f);

	outColor = max(0.15f, lightIntensity) * diffuseColor;
}
