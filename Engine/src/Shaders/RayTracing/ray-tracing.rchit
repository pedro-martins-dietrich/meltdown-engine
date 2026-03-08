#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_16bit_storage : enable

#include "utils.glsl"


hitAttributeEXT vec2 attributes;

layout(location = 0) rayPayloadInEXT Payload payload;

layout(push_constant) uniform PushConstant
{
	uint samplesPerPixel;
	uint maxRecursionDepth;
	uint maxScatterRayCount;
	uint accumulatedFrames;
	uint randomSeed;
} renderingInfo;

layout(set = 1, binding = 0) uniform accelerationStructureEXT accelerationStructure;
layout(set = 1, binding = 3) buffer VertexBuffer
{
	Vertex vertices[];
};
layout(set = 1, binding = 4) buffer IndexBuffer
{
	uint indices[];
};
layout(set = 1, binding = 5) buffer MaterialIndexBuffer
{
	uint16_t materialIDs[];
};
layout(set = 1, binding = 6) buffer MaterialsFloatBuffer
{
	MaterialFloatAttributes materialFloatAttributes[];
};


vec3 unitSphereSample(inout uint randomState)
{
	float z = 2 * randomFloat(randomState) - 1.0f;
	float r = sqrt(1.0f - z * z);
	float phi = TWO_PI * randomFloat(randomState);

	return vec3(r * cos(phi), r * sin(phi), z);
}

void getTriangleHitInfo(out vec3 normal, out vec3 hitPoint)
{
	vec3 v0 = vertices[indices[3 * gl_PrimitiveID]].position;
	vec3 v1 = vertices[indices[3 * gl_PrimitiveID + 1]].position;
	vec3 v2 = vertices[indices[3 * gl_PrimitiveID + 2]].position;
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;

	normal = cross(e1, e2);
	normal = normalize((gl_HitKindEXT == gl_HitKindFrontFacingTriangleEXT) ? normal : -normal);
	hitPoint = v0 + attributes.x * e1 + attributes.y * e2;
}

uint computeBounceRaysCount(float scatteringFactor)
{
	float baseCount = mix(1.0f, renderingInfo.maxScatterRayCount, scatteringFactor);
	return uint(round(baseCount));
}

float ggxNormalDistributionFunction(float normalDotHalfway, float alphaSquared)
{
	float squaredNH = normalDotHalfway * normalDotHalfway;
	float c = squaredNH * alphaSquared + (1.0f - squaredNH);

	return min(alphaSquared / (PI * c * c), 10.0f);
}

vec3 fresnelReflectanceSchlick(float cosTheta, vec3 f0)
{
	return f0 + (vec3(1.0f) - f0) * pow(1.0f - cosTheta, 5.0f);
}

float geometryFactorSchlickGGX(float normalDotRay, float halfAlpha)
{
	return normalDotRay / (normalDotRay * (1.0f - halfAlpha) + halfAlpha);
}

float geometryTermSmith(vec3 inRay, vec3 outRay, vec3 normal, float roughness)
{
	float k = 0.5f * roughness + 0.5f;
	k = 0.5f * k * k;

	return geometryFactorSchlickGGX(dot(normal, inRay), k)
		* geometryFactorSchlickGGX(dot(normal, outRay), k);
}

vec3 microfacetBRDF(vec3 inRay, vec3 outRay, vec3 normal, MaterialFloatAttributes material)
{
	vec3 halfwayVector = normalize(inRay + outRay);
	float alpha = max(material.roughness * material.roughness, 1e-6f);

	float normalDotIn = max(dot(normal, inRay), 1e-6f);
	float normalDotOut = max(dot(normal, outRay), 1e-6f);
	float normalDotHalfway = max(dot(normal, halfwayVector), 1e-6f);
	float halfwayDotIn = max(dot(halfwayVector, inRay), 1e-6f);

	vec3 baseColor = material.diffuse.rgb;

	float normalDistribution = ggxNormalDistributionFunction(normalDotHalfway, alpha * alpha);
	vec3 fresnel = fresnelReflectanceSchlick(halfwayDotIn, mix(F0, baseColor, material.metallic));
	float geometryTerm = geometryTermSmith(inRay, outRay, normal, material.roughness);

	vec3 diffuseColor = baseColor * (vec3(1.0f) - fresnel) * (1.0f - material.metallic) * INV_PI;

	vec3 specularColor = (normalDistribution * fresnel * geometryTerm)
		/ (4.0f * normalDotIn * normalDotOut);

	return diffuseColor + specularColor;
}

void main()
{
	payload.recursionDepth++;
	if(payload.recursionDepth >= renderingInfo.maxRecursionDepth)
	{
		payload.throughput = vec3(0.0f);
		return;
	}

	vec3 normal, hitPoint;
	getTriangleHitInfo(normal, hitPoint);

	MaterialFloatAttributes material = materialFloatAttributes[uint(materialIDs[gl_PrimitiveID])];
	float scatteringFactor = 1.0f - material.metallic * (1.0f - material.roughness);
	scatteringFactor = clamp(scatteringFactor, 0.0015f, 1.0f);
	vec3 perfectReflectionDirection = reflect(gl_WorldRayDirectionEXT, normal);

	Payload savedPayload = payload;

	uint bounceCount = computeBounceRaysCount(scatteringFactor);
	vec3 accumulatedLight = vec3(0.0f);
	for(uint bounceIndex = 0; bounceIndex < bounceCount; bounceIndex++)
	{
		vec3 randomDirection = unitSphereSample(payload.randomState);
		if(dot(normal, randomDirection) <= 0.0f)
			randomDirection = -randomDirection;

		vec3 bounceDirection = normalize(mix(perfectReflectionDirection, randomDirection, scatteringFactor));

		payload.light = vec3(0.0f);

		vec3 valueBRDF = microfacetBRDF(-gl_WorldRayDirectionEXT, bounceDirection, normal, material);
		payload.throughput *= valueBRDF * dot(bounceDirection, normal);

		traceRayEXT
		(
			accelerationStructure,
			gl_RayFlagsOpaqueEXT,
			0xFF,
			0, 0,
			0,
			hitPoint, 1e-3f,
			bounceDirection, 1e6f,
			0
		);

		accumulatedLight += payload.light * payload.throughput;

		payload.throughput = savedPayload.throughput;
		payload.recursionDepth = savedPayload.recursionDepth;
	}

	payload.light += (accumulatedLight / float(bounceCount))
		+ (payload.throughput * material.emissionAndIoR.rgb);
}
