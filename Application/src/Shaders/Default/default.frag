#version 450

struct MaterialData
{
	uint diffuseIndex;
	uint unused0;
	uint unused1;
	uint unused2;
};

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) flat in uint materialIdOffset;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 3) uniform sampler2D textures[1024];
layout(set = 0, binding = 4) readonly buffer Materials
{
	MaterialData materialData[];
};
layout(set = 0, binding = 5) readonly buffer MaterialIndexing
{
	uint materialOffsets[];
};
layout(set = 0, binding = 6) readonly buffer MaterialSets
{
	uint materialIDs[];
};

layout(set = 2, binding = 0) uniform LightData
{
	vec3 lightDirection;
	float ambientLightIntensity;
} lightData;

void main()
{
	uint materialID = materialIDs[materialIdOffset];
	MaterialData material = materialData[materialOffsets[materialID]];

	float lightIntensity = 0.8f * (dot(fragNormal, -lightData.lightDirection) + 0.25f);

	outColor = max(lightData.ambientLightIntensity, lightIntensity) * texture(textures[material.diffuseIndex], fragUV);
}
