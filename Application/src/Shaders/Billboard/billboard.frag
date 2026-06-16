#version 450

struct MaterialData
{
	uint diffuseIndex;
	uint unused0;
	uint unused1;
	uint unused2;
};

layout(location = 0) in vec2 fragUV;
layout(location = 1) flat in uint materialIdOffset;

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

void main()
{
	uint materialID = materialIDs[materialIdOffset];
	MaterialData material = materialData[materialOffsets[materialID]];

	outColor = texture(textures[material.diffuseIndex], fragUV);
}
