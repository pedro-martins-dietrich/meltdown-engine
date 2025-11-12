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
	vec4 emissionAndIoR;
	float roughness;
	float metallic;
};


const float PI = 3.14159265359f;
const float TWO_PI = 6.28318530718f;
const float INV_PI = 0.31830988618f;

const vec3 F0 = vec3(0.04f);


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
	float z = 2 * randomFloat(randomState) - 1.0f;
	float r = sqrt(1.0f - z * z);
	float phi = TWO_PI * randomFloat(randomState);

	return vec3(r * cos(phi), r * sin(phi), z);
}

float ggxNormalDistributionFunction(float normalDotHalfway, float alphaSquared)
{
	float c = normalDotHalfway * normalDotHalfway * (alphaSquared - 1.0f) + 1.0f;

	return clamp(alphaSquared / (PI * c * c), 0.0f, 1.0f);
}

vec3 fresnelReflectanceSchlick(float cosTheta, in vec3 f0)
{
	return f0 + (vec3(1.0f) - f0) * pow(1.0f - cosTheta, 5U);
}

float geometryFactorSchlickGGX(float normalDotRay, float halfAlpha)
{
	return normalDotRay / (normalDotRay * (1.0f - halfAlpha) + halfAlpha);
}

float geometryTermSmith(in vec3 inRay, in vec3 outRay, in vec3 normal, float roughness)
{
	float k = 0.5f * roughness + 0.5f;
	k *= 0.5f * k;

	return geometryFactorSchlickGGX(dot(normal, inRay), k)
		* geometryFactorSchlickGGX(dot(normal, outRay), k);
}

vec3 microfacetBRDF(in vec3 inRay, in vec3 outRay, in vec3 normal, in MaterialFloatAttributes material)
{
	vec3 halfwayVector = normalize(inRay + outRay);
	float alpha = max(1e-6, material.roughness * material.roughness);

	vec3 baseColor = material.diffuse.rgb;

	float normalDistribution = ggxNormalDistributionFunction(dot(normal, halfwayVector), alpha * alpha);
	vec3 fresnel = fresnelReflectanceSchlick(dot(inRay, halfwayVector), mix(F0, baseColor, material.metallic));
	float geometryTerm = geometryTermSmith(inRay, outRay, normal, material.roughness);

	vec3 diffuseColor = baseColor * (vec3(1.0f) - fresnel) * (1.0f - material.metallic) * INV_PI;

	vec3 specularColor = (normalDistribution * fresnel * geometryTerm)
		/ (4.0f * dot(normal, inRay) * dot(normal, outRay));

	return diffuseColor + specularColor;
}

void main()
{
	payload.recursionDepth++;
	if(payload.recursionDepth >= renderingInfo.maxRecursionDepth) return;

	vec3 v0 = vertices[indices[3 * gl_PrimitiveID]].position;
	vec3 v1 = vertices[indices[3 * gl_PrimitiveID + 1]].position;
	vec3 v2 = vertices[indices[3 * gl_PrimitiveID + 2]].position;
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;

	vec3 normal = cross(e1, e2);
	normal = normalize((gl_HitKindEXT == gl_HitKindFrontFacingTriangleEXT) ? normal : -normal);
	vec3 hitPoint = v0 + attributes.x * e1 + attributes.y * e2;

	MaterialFloatAttributes material = materialFloatAttributes[uint(materialIDs[gl_PrimitiveID])];

	vec3 diffuseDirection = unitSphereSample(payload.randomState);
	if(dot(normal, diffuseDirection) <= 0.0f)
		diffuseDirection = -diffuseDirection;
	vec3 specularReflection = reflect(gl_WorldRayDirectionEXT, normal);
	vec3 newDirection = normalize(mix(specularReflection, diffuseDirection, material.roughness));

	vec3 valueBRDF = microfacetBRDF(-gl_WorldRayDirectionEXT, newDirection, normal, material);

	payload.light += material.emissionAndIoR.xyz * payload.throughput;
	payload.throughput *= valueBRDF * dot(newDirection, normal);

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
