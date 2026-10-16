#version 460
#extension GL_EXT_ray_tracing : enable

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

layout(set = 0, binding = 1) readonly buffer VertexBuffer
{
	Vertex vertices[];
};
layout(set = 0, binding = 2) readonly buffer IndexBuffer
{
	uint indices[];
};
layout(set = 0, binding = 3) readonly buffer Submeshes
{
	Submesh submeshes[];
};
layout(set = 0, binding = 4) readonly buffer Materials
{
	MaterialData materialDataNew[];
};
layout(set = 0, binding = 5) readonly buffer MaterialIndexing
{
	uint materialOffsets[];
};
layout(set = 0, binding = 6) readonly buffer MaterialSets
{
	uint materialIDsNew[];
};
layout(set = 0, binding = 7) readonly buffer RenderObjects
{
	RenderObject instances[];
};
layout(set = 0, binding = 8) uniform accelerationStructureEXT accelerationStructure;


vec3 unitSphereSample(inout uint randomState)
{
	float z = 2.0f * randomFloat(randomState) - 1.0f;
	float r = sqrt(1.0f - z * z);
	float phi = TWO_PI * randomFloat(randomState);

	return vec3(r * cos(phi), r * sin(phi), z);
}

void getTriangleHitInfo(uint submeshIndexOffset, out vec3 normal)
{
	uint indexBase = submeshIndexOffset + 3U * gl_PrimitiveID;
	vec3 v0 = vertices[indices[indexBase]].position.xyz;
	vec3 v1 = vertices[indices[indexBase + 1]].position.xyz;
	vec3 v2 = vertices[indices[indexBase + 2]].position.xyz;
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;

	vec3 objectSpaceNormal = cross(e1, e2);

	mat3 normalMatrix = transpose(mat3(gl_WorldToObjectEXT));
	normal = normalize(normalMatrix * objectSpaceNormal);
	normal = normalize((gl_HitKindEXT == gl_HitKindFrontFacingTriangleEXT) ? normal : -normal);
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

	return geometryFactorSchlickGGX(dot(normal, inRay), k) * geometryFactorSchlickGGX(dot(normal, outRay), k);
}

vec3 microfacetBRDF(vec3 inRay, vec3 outRay, vec3 normal, MaterialData material)
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

	vec3 specularColor = (normalDistribution * fresnel * geometryTerm) / (4.0f * normalDotIn * normalDotOut);

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

	RenderObject instance = instances[gl_InstanceCustomIndexEXT];
	Submesh submesh = submeshes[instance.submeshOffset + gl_GeometryIndexEXT];

	vec3 hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	vec3 normal;
	getTriangleHitInfo(submesh.indexOffset, normal);

	uint materialID = materialIDsNew[instance.materialSetID + submesh.materialSlot];
	MaterialData material = materialDataNew[materialOffsets[materialID] / 3U];

	float scatteringFactor = 1.0f - material.metallic * (1.0f - material.roughness);
	scatteringFactor = clamp(scatteringFactor, 0.0015f, 1.0f);
	vec3 perfectReflectionDirection = reflect(gl_WorldRayDirectionEXT, normal);

	Payload savedPayload = payload;

	uint bounceCount = computeBounceRaysCount(scatteringFactor);
	vec3 accumulatedLight = vec3(0.0f);

	for(uint bounceIndex = 0U; bounceIndex < bounceCount; bounceIndex++)
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

	payload.light += (accumulatedLight / float(bounceCount)) + (payload.throughput * material.emissionAndIoR.rgb);
}
