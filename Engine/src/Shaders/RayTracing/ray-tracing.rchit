#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_16bit_storage : enable

struct Payload
{
	vec3 light;
	vec3 throughput;
	uint recursionDepth;
	uint randomState;
};

struct Vertex
{
	vec3 position;
	vec2 textureUV;
};

struct MaterialFloatAttributes
{
	vec4 diffuse;
};


const float PI = 3.14159265359f;
const float TWO_PI = 6.28318530718f;


hitAttributeEXT vec2 attributes;

layout(location = 0) rayPayloadInEXT Payload payload;

layout(push_constant) uniform PushConstant
{
	uint maxRecursionDepth;
	uint samplesPerPixel;
	uint accumulatedFrames;
	uint randomSeed;
} renderingInfo;

layout(set = 1, binding = 0) uniform accelerationStructureEXT accelerationStructure;
layout(set = 1, binding = 2) buffer VertexBuffer
{
	Vertex vertices[];
};
layout(set = 1, binding = 3) buffer IndexBuffer
{
	uint indices[];
};
layout(set = 1, binding = 4) buffer MaterialIndexBuffer
{
	uint16_t materialIDs[];
};
layout(set = 1, binding = 5) buffer MaterialsFloatBuffer
{
	MaterialFloatAttributes materialFloatAttributes[];
};


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

vec3 unitSphereSample(inout uint randomState)
{
	float theta = randomFloat(randomState) * TWO_PI;
	float phi = (randomFloat(randomState) - 0.5f) * PI;

	return vec3(cos(phi) * cos(theta), -sin(phi), cos(phi) * sin(theta));
}

void main()
{
	payload.recursionDepth++;

	vec3 v0 = vertices[indices[3 * gl_PrimitiveID]].position;
	vec3 v1 = vertices[indices[3 * gl_PrimitiveID + 1]].position;
	vec3 v2 = vertices[indices[3 * gl_PrimitiveID + 2]].position;
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;

	vec3 normal = cross(e1, e2);
	normal = normalize((gl_HitKindEXT == gl_HitKindFrontFacingTriangleEXT) ? normal : -normal);
	vec3 hitPoint = v0 + attributes.x * e1 + attributes.y * e2;

	vec3 newDirection = unitSphereSample(payload.randomState);
	newDirection = normalize(newDirection + normal);

	MaterialFloatAttributes material = materialFloatAttributes[uint(materialIDs[gl_PrimitiveID])];
	float emissivity = 0.0f;

	payload.light += emissivity * payload.throughput;
	payload.throughput *= material.diffuse.xyz;

	if(payload.recursionDepth >= renderingInfo.maxRecursionDepth) return;
	traceRayEXT
	(
		accelerationStructure,
		gl_RayFlagsOpaqueEXT,
		0xFF,
		0, 0,
		0,
		hitPoint, 0.001f,
		newDirection, 10000.0f,
		0
	);
}
