#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_shader_16bit_storage : enable

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
};

layout(location = 0) rayPayloadInEXT vec4 payload;

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

bool intersectTriangle(Ray ray, Vertex v0, Vertex v1, Vertex v2, out float hitDistance)
{
	vec3 edge1 = v1.position - v0.position;
	vec3 edge2 = v2.position - v0.position;

	vec3 hVector = cross(ray.direction, edge2);
	float det = dot(edge1, hVector);
	if(abs(det) < 1e-6) return false;

	float invDet = 1.0f / det;
	vec3 relativeRayOrigin = ray.origin - v0.position;
	float u = invDet * dot(relativeRayOrigin, hVector);
	if(u < -1e-6 || u - 1.0f > 1e-6) return false;

	vec3 qVector = cross(relativeRayOrigin, edge1);
	float v = invDet * dot(ray.direction, qVector);
	if(v < -1e-6 || u + v - 1.0f > 1e-6) return false;

	float t = invDet * dot(edge2, qVector);
	if(t < 1e-6) return false;

	hitDistance = t;
	return true;
}

void main()
{
	Ray ray;
	ray.origin = gl_WorldRayOriginEXT;
	ray.direction = gl_WorldRayDirectionEXT;

	float backgroundColor = 0.05f * (1.2f + dot(ray.direction.xyz, vec3(0.0f, -1.0f, 0.0f)));

	vec4 color = vec4(vec3(backgroundColor), 1.0f);
	float closestHit = 1e9;

	int triangleCount = indices.length() / 3;
	for(int i = 0; i < triangleCount; i++)
	{
		Vertex v0 = vertices[indices[3 * i]];
		Vertex v1 = vertices[indices[3 * i + 1]];
		Vertex v2 = vertices[indices[3 * i + 2]];

		float hitDistance = 1e10;
		if(intersectTriangle(ray, v0, v1, v2, hitDistance) && hitDistance < closestHit)
		{
			closestHit = hitDistance;
			color = materialFloatAttributes[uint(materialIDs[i])].diffuse;
		}
	}

	payload = color;
}
