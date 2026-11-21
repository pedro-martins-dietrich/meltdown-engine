#include <pch.hpp>
#include "BlasBuilder.hpp"

#include <numeric>

#include "../../Utils/Logger.hpp"

namespace mtd::BlasBuilder
{
	constexpr uint32_t LEAF_NODE = 0x80000000U;
	constexpr uint32_t NODE_INDEX_MASK = 0x7FFFFFFFU;
	constexpr uint32_t INVALID_NODE = 0xFFFFFFFFU;

	constexpr size_t WINDOW = 16;

	// Axis-Aligned Bounding Box
	struct AABB
	{
		Vec3 min;
		Vec3 max;

		inline float halfArea() const
		{
			Vec3 dimensions{max - min};
			return dimensions.x * dimensions.y + dimensions.y * dimensions.z + dimensions.x * dimensions.z;
		}
		inline Vec3 center() const { return 0.5f * (min + max); }
	};

	// Temporary node for the intermediate BVH2 tree building
	struct TempNode
	{
		AABB aabb;
		bool leaf = false;
		uint32_t left = INVALID_NODE;
		uint32_t right = INVALID_NODE;
		uint32_t triangles = INVALID_NODE;
	};

	// BVH4 data header
	struct alignas(16) Header
	{
		uint32_t nodeCount = 0U;
		uint32_t nodeOffset = 0U;
		uint32_t triangleCount = 0U;
		uint32_t triangleOffset = 0U;
		uint32_t rootReference = 0U;
		uint32_t padding[3] = {0U, 0U, 0U};
	};

	static inline AABB mergeAABB(const AABB& a, const AABB& b)
	{
		return {min(a.min, b.min), max(a.max, b.max)};
	}

	static inline Vec3 getVertex(const std::vector<std::byte>& vertices, size_t vertexOffset)
	{
		Vec3 vertex{0.0f};
		if(vertices.size() < vertexOffset + sizeof(Vec3))
		{
			LOG_ERROR("Vertex position out of bounds for a BLAS build.");
			return vertex;
		}
		std::memcpy(&vertex, vertices.data() + vertexOffset, sizeof(Vec3));
		return vertex;
	}

	// Expands the lowest 10 bits from `value` into 30 bits, alternating between 2 zeros and a bit from `value`
	static inline uint32_t spreadBits(uint32_t value)
	{
		value &= 0x000003FFU;
		value = (value | (value << 16U)) & 0x030000FFU;
		value = (value | (value << 8U)) & 0x0300F00FU;
		value = (value | (value << 4U)) & 0x030C30C3U;
		return (value | (value << 2U)) & 0x09249249U;
	}
	// Normalizes a value within the limits as an unsigned integer between 0 and 1023
	static inline uint32_t normalizeForMortonCode(float value, float min, float max)
	{
		float length = max - min;
		float normalized = std::clamp((value - min) / length, 0.0f, 1.0f);
		return static_cast<uint32_t>(normalized * 1023.0f);
	}
	// Converts a 3D coordinate within an AABB to an unsigned integer using the Morton code
	static inline uint32_t convertToMorton(Vec3 point, const AABB& limits)
	{
		uint32_t x = normalizeForMortonCode(point.x, limits.min.x, limits.max.x);
		uint32_t y = normalizeForMortonCode(point.y, limits.min.y, limits.max.y);
		uint32_t z = normalizeForMortonCode(point.z, limits.min.z, limits.max.z);
		return (spreadBits(x) << 2U) | (spreadBits(y) << 1U) | spreadBits(z);
	}

	static uint32_t emit(const std::vector<TempNode>& temp, uint32_t id, std::vector<NodeBVH4>& bvh4);
}

bool mtd::BlasBuilder::buildBLAS
(
	const MeshData& mesh, const std::vector<std::byte>& vertices, const std::vector<uint32_t>& indices,
	std::vector<std::byte>& blasData
)
{
	if(mesh.vertexStride < 3 * sizeof(float))
	{
		LOG_ERROR("Vertex format not suitable to build a BLAS.");
		return false;
	}

	Header header{};
	std::vector<NodeBVH4> bvh4;
	std::vector<TriangleBVH> triangles;
	std::vector<TempNode> temp;

	for(const SubmeshData& submesh: mesh.submeshes)
	{
		if(submesh.indexCount % 3U != 0U || submesh.indexOffset + submesh.indexCount > indices.size())
		{
			LOG_ERROR("Invalid submesh index count. Cannot build the mesh BLAS.");
			return false;
		}
		for(size_t k = 0; k < submesh.indexCount; k += 3)
		{
			UIntVec3 triangleIndices =
			{
				indices[submesh.indexOffset + k],
				indices[submesh.indexOffset + k + 1],
				indices[submesh.indexOffset + k + 2]
			};
			Vec3 vertex0 = getVertex(vertices, mesh.vertexOffset + triangleIndices.x * mesh.vertexStride);
			Vec3 vertex1 = getVertex(vertices, mesh.vertexOffset + triangleIndices.y * mesh.vertexStride);
			Vec3 vertex2 = getVertex(vertices, mesh.vertexOffset + triangleIndices.z * mesh.vertexStride);

			AABB aabb{min(vertex0, min(vertex1, vertex2)), max(vertex0, max(vertex1, vertex2))};
			temp.emplace_back(aabb, true, INVALID_NODE, INVALID_NODE, static_cast<uint32_t>(triangles.size()));
			triangles.emplace_back(triangleIndices, submesh.materialSlot);
		}
	}
	if(temp.empty())
	{
		LOG_ERROR("Failed to build triangles AABB for the mesh BVH.");
		return false;
	}

	std::vector<uint32_t> activeNodes(temp.size());
	std::iota(activeNodes.begin(), activeNodes.end(), 0U);
	AABB meshAABB{mesh.centerAABB - mesh.extentAABB, mesh.centerAABB + mesh.extentAABB};
	std::stable_sort(activeNodes.begin(), activeNodes.end(), [&](uint32_t a, uint32_t b)
	{
		return convertToMorton(temp[a].aabb.center(), meshAABB) < convertToMorton(temp[b].aabb.center(), meshAABB);
	});

	while(activeNodes.size() > 1)
	{
		size_t activeCount = activeNodes.size();
		std::vector<uint32_t> nearest(activeCount, INVALID_NODE);
		for(size_t i = 0; i < activeCount; i++)
		{
			size_t begin = (i > WINDOW) ? i - WINDOW : 0;
			size_t end = std::min(activeCount - 1, i + WINDOW);
			float best = std::numeric_limits<float>::infinity();
			size_t bestDistance = std::numeric_limits<size_t>::max();
			size_t preferred = i ^ 1;

			for(size_t j = begin; j <= end; j++)
			{
				if(i == j) continue;
				float score = mergeAABB(temp[activeNodes[i]].aabb, temp[activeNodes[j]].aabb).halfArea();
				size_t distance = (i > j) ? (i - j) : (j - i);
				if(score < best || (score == best && nearest[i] != preferred && (j == preferred || (j != preferred && distance < bestDistance))))
				{
					best = score;
					nearest[i] = static_cast<uint32_t>(j);
					bestDistance = distance;
				}
			}
		}

		std::vector<uint32_t> next;
		next.reserve(activeCount);
		uint32_t mergedNodesCount = 0U;
		for(size_t i = 0; i < activeCount; i++)
		{
			size_t j = nearest[i];
			if(nearest[j] != i || j == INVALID_NODE)
			{
				next.push_back(activeNodes[i]);
				continue;
			}
			if(i > j) continue;
			if(temp.size() > NODE_INDEX_MASK)
			{
				LOG_ERROR("Exceeded the node count limit (%d) while building the mesh BVH4.", NODE_INDEX_MASK);
				return false;
			}
			uint32_t nodeA = activeNodes[i];
			uint32_t nodeB = activeNodes[j];
			next.push_back(static_cast<uint32_t>(temp.size()));
			temp.emplace_back(mergeAABB(temp[nodeA].aabb, temp[nodeB].aabb), false, nodeA, nodeB, INVALID_NODE);
			mergedNodesCount++;
		}
		if(mergedNodesCount == 0U)
		{
			LOG_ERROR("Failed to make progress while building the mesh BVH4 with PLOC.");
			return false;
		}
		activeNodes.swap(next);
	}
	header.rootReference = emit(temp, activeNodes[0], bvh4);
	header.nodeCount = static_cast<uint32_t>(bvh4.size());
	header.nodeOffset = sizeof(Header) / sizeof(uint32_t);
	header.triangleCount = static_cast<uint32_t>(triangles.size());

	size_t triangleOffset = sizeof(Header) + bvh4.size() * sizeof(NodeBVH4);
	header.triangleOffset = triangleOffset / sizeof(uint32_t);

	size_t totalDataSize = triangleOffset + triangles.size() * sizeof(TriangleBVH);
	blasData.resize(totalDataSize);
	std::memcpy(blasData.data(), &header, sizeof(Header));
	std::memcpy(blasData.data() + sizeof(Header), bvh4.data(), bvh4.size() * sizeof(NodeBVH4));
	std::memcpy(blasData.data() + triangleOffset, triangles.data(), triangles.size() * sizeof(TriangleBVH));

	LOG_INFO("Created mesh BLAS.");
	return true;
}

uint32_t mtd::BlasBuilder::emit(const std::vector<TempNode>& temp, uint32_t id, std::vector<NodeBVH4>& bvh4)
{
	const TempNode& tNode = temp[id];
	if(tNode.leaf)
		return LEAF_NODE | tNode.triangles;

	std::array<uint32_t, 4> children{};
	children[0] = tNode.left;
	children[1] = tNode.right;
	children[2] = INVALID_NODE;
	children[3] = INVALID_NODE;

	size_t childCount = 2;
	while(childCount < 4)
	{
		size_t chosen = 4;
		float largest = -1.0f;
		for(size_t k = 0; k < childCount; k++)
		{
			const TempNode& childNode = temp[children[k]];
			if(childNode.leaf) continue;

			float halfArea = childNode.aabb.halfArea();
			if(chosen == 4 || halfArea > largest)
			{
				chosen = k;
				largest = halfArea;
			}
		}
		if(chosen == 4) break;
		uint32_t child = children[chosen];
		children[chosen] = temp[child].left;
		children[childCount++] = temp[child].right;
	}

	uint32_t newNodeReference = static_cast<uint32_t>(bvh4.size());
	bvh4.emplace_back();
	for(size_t k = 0; k < childCount; k++)
	{
		uint32_t child = children[k];
		uint32_t nodeRef = emit(temp, child, bvh4);
		bvh4[newNodeReference].children[k] = ChildBVH{temp[child].aabb.min, nodeRef, temp[child].aabb.max};
	}
	return newNodeReference;
}
