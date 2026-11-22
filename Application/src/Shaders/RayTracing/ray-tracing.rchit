#version 460
#extension GL_EXT_ray_tracing : enable

#include "utils.glsl"


hitAttributeEXT vec2 attributes;

layout(location = 0) rayPayloadInEXT Payload payload;

layout(push_constant) uniform PushConstant
{
	uint samplesPerPixel;
	uint maxRecursionDepth;
	uint accumulatedFrames;
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
layout(set = 0, binding = 7) readonly buffer RenderObjects
{
	RenderObject instances[];
};
layout(set = 0, binding = 8) uniform accelerationStructureEXT accelerationStructure;


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

void main()
{
	RenderObject instance = instances[gl_InstanceCustomIndexEXT];
	Submesh submesh = submeshes[instance.submeshOffset + gl_GeometryIndexEXT];

	vec3 normal;
	getTriangleHitInfo(submesh.indexOffset, normal);

	uint materialID = materialIDs[instance.materialSetID + submesh.materialSlot];
	MaterialData material = materialData[materialOffsets[materialID] / 3U];

	payload.light += payload.throughput * material.emissionAndIoR.rgb;

	if(payload.recursionDepth < renderingInfo.maxRecursionDepth)
	{
		SampleData sampleData = sampleBRDF
		(
			-gl_WorldRayDirectionEXT, normal, payload.randomState,
			material.diffuse.rgb, material.roughness, material.metallic
		);

		if(!sampleData.valid) return;

		Ray newRay;
		newRay.origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
		newRay.direction = sampleData.newRayDirection;

		payload.throughput *= sampleData.weight;
		payload.recursionDepth++;

		if(russianRoulette(payload.recursionDepth, payload.throughput, payload.randomState)) return;

		traceRayEXT
		(
			accelerationStructure,
			gl_RayFlagsOpaqueEXT,
			0xFF,
			0, 0,
			0,
			newRay.origin, 1e-3f,
			newRay.direction, 1e6f,
			0
		);
	}
}
