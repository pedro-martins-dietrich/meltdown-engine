struct Payload
{
	vec3 light;
	vec3 throughput;
	uint recursionDepth;
	uint randomState;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct Vertex
{
	vec3 position;
	vec2 textureUV;
};

struct MaterialFloatAttributes
{
	vec4 diffuse;
	vec4 emissionAndIoR;
	float roughness;
	float metallic;
};


const float PI = 3.14159265359f;
const float TWO_PI = 6.28318530718f;
const float INV_PI = 0.31830988618f;

const vec3 F0 = vec3(0.04f);


uint pcgHash(uint inputValue)
{
	uint state = inputValue * 0x2C9277B5U + 0xAC564B05U;
	uint word = ((state >> ((state >> 28U) + 4U)) ^ state) * 0x108EF2D9U;
	return (word >> 22U) ^ word;
}

float randomFloat(inout uint seed)
{
	seed = pcgHash(seed);
	return float(seed) / float(0xFFFFFFFFU);
}
