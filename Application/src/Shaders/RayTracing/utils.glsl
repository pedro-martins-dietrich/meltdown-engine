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
	vec4 position;
	vec4 textureUV;
};

struct AABB
{
	vec3 center;
	vec3 extent;
};

struct Submesh
{
	uint indexOffset;
	uint indexCount;
	uint materialSlot;
	AABB aabb;
};

struct RenderObject
{
	mat4 transform;
	uint materialSetID;
	uint submeshOffset;
	uint submeshCount;
	uint vertexOffset;
	AABB aabb;
};

struct MaterialData
{
	vec4 diffuse;
	vec4 emissionAndIoR;
	float roughness;
	float metallic;
};

struct SampleData
{
	vec3 weight;
	vec3 newRayDirection;
	bool valid;
};


const float PI = 3.14159265359f;
const float TWO_PI = 6.28318530718f;
const float INV_PI = 0.31830988618f;

const vec3 DIELETRIC_F0 = vec3(0.04f);
const vec3 PERCEIVED_LUMINANCE = vec3(0.2126f, 0.7152f, 0.0722f);

const uint RUSSIAN_ROULETTE_MIN_DEPTH = 3U;
const float RUSSIAN_ROULETTE_MAX_TERMINATION = 0.95f;


uint pcgHash(uint inputValue)
{
	uint state = inputValue * 0x2C9277B5U + 0xAC564B05U;
	uint word = ((state >> ((state >> 28U) + 4U)) ^ state) * 0x108EF2D9U;
	return (word >> 22U) ^ word;
}

uint initializeRandomSeed(uint pixelIndex, uint frameIndex)
{
	uint seed = pcgHash(frameIndex ^ 0x9E3779B9U);
	return pcgHash(pixelIndex ^ seed);
}

float randomFloat(inout uint seed)
{
	seed = pcgHash(seed);
	return float(seed >> 8U) * 5.9604644775390625e-8;
}

vec3 unitSphereSample(inout uint randomState)
{
	float z = 2.0f * randomFloat(randomState) - 1.0f;
	float radius = sqrt(1.0f - z * z);
	float phi = TWO_PI * randomFloat(randomState);

	return vec3(radius * cos(phi), radius * sin(phi), z);
}

vec3 cosineWeightedHemisphereSample(inout uint randomState)
{
	float sinThetaSquared = randomFloat(randomState);
	float radius = sqrt(sinThetaSquared);
	float phi = TWO_PI * randomFloat(randomState);

	return vec3(radius * cos(phi), radius * sin(phi), sqrt(1.0f - sinThetaSquared));
}

float specularReflectionProbability(vec3 diffuseColor, vec3 f0)
{
	float specularStrength = dot(f0, PERCEIVED_LUMINANCE);
	float diffuseStrength = dot(diffuseColor * (vec3(1.0f) - f0), PERCEIVED_LUMINANCE);

	float totalStrength = specularStrength + diffuseStrength;
	return specularStrength / totalStrength;
}

vec3 localNormalToWorldNormal(vec3 localNormal, vec3 worldNormal)
{
	vec3 tangent = normalize
	(
		(abs(worldNormal.z) < 0.999f)
		? cross(vec3(0.0f, 0.0f, 1.0f), worldNormal)
		: cross(vec3(0.0f, 1.0f, 0.0f), worldNormal)
	);
	vec3 bitangent = cross(worldNormal, tangent);
	return normalize(tangent * localNormal.x + bitangent * localNormal.y + worldNormal * localNormal.z);
}

vec3 sampleMicrofacetNormalGGX(vec3 surfaceNormal, float alphaSquared, inout uint randomState)
{
	float x = randomFloat(randomState);
	float cosThetaSquared = (1.0f - x) / (x * (alphaSquared - 1.0f) + 1.0f);
	float cosTheta = sqrt(cosThetaSquared);
	float sinTheta = sqrt(max(0.0f, 1.0f - cosThetaSquared));
	float phi = TWO_PI * randomFloat(randomState);

	vec3 localMicrofacetNormal = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
	return localNormalToWorldNormal(localMicrofacetNormal, surfaceNormal);
}

float normalDistribuitionFunctionGGX(float alphaSquared, float normalDotHalf)
{
	float d = normalDotHalf * normalDotHalf * (alphaSquared - 1.0f) + 1.0f;
	return alphaSquared / (PI * d * d);
}

vec3 fresnelSchlick(vec3 albedo, float metallic, vec3 f0, float inDotHalf)
{
	float x = 1.0f - inDotHalf;
	float xSquared = x * x;

	return f0 + (vec3(1.0f) - f0) * (xSquared * xSquared * x);
}

float geometryFactorSchlickGGX(float halfAlpha, float normalDotRayDirection)
{
	return normalDotRayDirection / (normalDotRayDirection * (1.0f - halfAlpha) + halfAlpha);
}

float geometrySchlickGGX(float halfAlpha, float normalDotIn, float normalDotOut)
{
	return geometryFactorSchlickGGX(halfAlpha, normalDotIn) * geometryFactorSchlickGGX(halfAlpha, normalDotOut);
}

SampleData sampleBRDF
(
	vec3 incomingRayDirection, vec3 normalDirection, inout uint randomState,
	vec3 albedo, float roughness, float metallic
)
{
	SampleData result;
	result.valid = false;

	vec3 diffuseColor = (1.0f - metallic) * albedo;
	vec3 f0 = mix(DIELETRIC_F0, albedo, metallic);

	float specularProbability = specularReflectionProbability(diffuseColor, f0);

	float alpha = clamp(roughness * roughness, 0.0004f, 1.0f);
	float alphaSquared = alpha * alpha;

	vec3 outgoingRayDirection;
	if(randomFloat(randomState) < specularProbability)
	{
		vec3 microfacetNormal = sampleMicrofacetNormalGGX(normalDirection, alphaSquared, randomState);
		outgoingRayDirection = reflect(-incomingRayDirection, microfacetNormal);
	}
	else
	{
		vec3 localDiffuseBounceDirection = cosineWeightedHemisphereSample(randomState);
		outgoingRayDirection = localNormalToWorldNormal(localDiffuseBounceDirection, normalDirection);
	}

	float normalDotOut = dot(normalDirection, outgoingRayDirection);
	if(normalDotOut <= 0.0f)
		return result;

	float normalDotIn = dot(normalDirection, incomingRayDirection);
	vec3 halfVector = normalize(incomingRayDirection + outgoingRayDirection);
	float normalDotHalf = dot(normalDirection, halfVector);
	float inDotHalf = dot(incomingRayDirection, halfVector);

	float normalDistribution = normalDistribuitionFunctionGGX(alphaSquared, normalDotHalf);

	float diffusePDF = normalDotOut * INV_PI;
	float specularPDF = (normalDotHalf * normalDistribution) / (4.0f * inDotHalf);
	float brdfPDF = specularProbability * specularPDF + (1.0f - specularProbability) * diffusePDF;

	vec3 fresnelTerm = fresnelSchlick(albedo, metallic, f0, inDotHalf);
	float geometryTerm = geometrySchlickGGX(0.5f * alpha, normalDotIn, normalDotOut);

	vec3 specularBRDF = (normalDistribution * fresnelTerm * geometryTerm) / (4.0f * normalDotIn * normalDotOut);
	vec3 diffuseBRDF = diffuseColor * (vec3(1.0f) - fresnelTerm) * INV_PI;

	vec3 brdf = specularBRDF + diffuseBRDF;

	result.valid = true;
	result.weight = brdf * normalDotOut / brdfPDF;
	result.newRayDirection = outgoingRayDirection;
	return result;
}

bool russianRoulette(uint rayRecursionDepth, inout vec3 throughput, inout uint randomState)
{
	if(rayRecursionDepth < RUSSIAN_ROULETTE_MIN_DEPTH)
		return false;

	float importance = max(throughput.r, max(throughput.g, throughput.b));
	float terminationProbability = clamp(1.0f - importance, 0.0f, RUSSIAN_ROULETTE_MAX_TERMINATION);
	if(randomFloat(randomState) < terminationProbability)
		return true;

	throughput /= (1.0f - terminationProbability);

	return false;
}

void reinhardToneMapping(inout vec3 color)
{
	color /= vec3(1.0f) + color;
}
