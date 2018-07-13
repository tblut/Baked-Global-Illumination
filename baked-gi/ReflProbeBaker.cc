#include "ReflProbeBaker.hh"
#include "RenderPipeline.hh"
#include "PathTracer.hh"
#include "Scene.hh"
#include "TriBoxTest.hh"

#include <glow/common/log.hh>
#include <limits>
#include <random>
#include <algorithm>
#include <numeric>

/*
 TODO:
 Algorithm for placing probes in the scene and assigning probes to vertices:
 
 1. Voxelize scene (make 3D grid)
 2. Determine "empty", "full" and "surface" voxels
    - "empty" = No vertex inside
    - "full" = At least one vertex is too close to the center of the voxel
    - "surface" = At least one vertex is in the voxel but they are all not too close too the center
 3. Generate env maps for every empty voxel
 4. For every geom voxel shoot N reflection rays and record how many intersection points can be seen by every probe
 5. For every surface voxel choose the K best probes
 6. For every vertex in the scene get the according voxel and assign the K probes
 */

namespace {
    bool isPointInsideBox(glm::vec3 point, glm::vec3 boxMin, glm::vec3 boxMax) {
        return point.x > boxMin.x && point.x < boxMax.x
            && point.y > boxMin.y && point.y < boxMax.y
            && point.z > boxMin.z && point.z < boxMax.z;
    }

	bool testTriangleAABB(glm::vec3 triA, glm::vec3 triB, glm::vec3 triC, glm::vec3 aabbMin, glm::vec3 aabbMax) {
		float boxcenter[3] = { (aabbMin.x + aabbMax.x) * 0.5f, (aabbMin.y + aabbMax.y) * 0.5f, (aabbMin.z + aabbMax.z) * 0.5f };
		float boxhalfsize[3] = { (aabbMax.x - aabbMin.x) * 0.5f, (aabbMax.y - aabbMin.y) * 0.5f, (aabbMax.z - aabbMin.z) * 0.5f };
		float triverts[3][3] = {
			{ triA.x, triA.y, triA.z },
			{ triB.x, triB.y, triB.z },
			{ triC.x, triC.y, triC.z }
		};
		return triBoxOverlap(boxcenter, boxhalfsize, triverts) == 1;
	}

	bool isPointInsidePrimitve(const Primitive& primitive, glm::vec3 point) {
		for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
			glm::vec3 A = primitive.positions[primitive.indices[i]];
			glm::vec3 B = primitive.positions[primitive.indices[i + 1]];
			glm::vec3 C = primitive.positions[primitive.indices[i + 2]];

			A = primitive.transform * glm::vec4(A, 1.0f);
			B = primitive.transform * glm::vec4(B, 1.0f);
			C = primitive.transform * glm::vec4(C, 1.0f);

			glm::vec3 v0 = B - A;
			glm::vec3 v1 = C - A;
			glm::vec3 N = glm::normalize(glm::cross(v0, v1));
			glm::vec3 triCenter = (v0 + v1) * 0.5f;

			if (glm::dot(N, point - triCenter) > 0.0f) {
				return false;
			}
		}

		return true;
	}

	bool isBoxInsidePrimitve(const Primitive& primitive, glm::vec3 aabbMin, glm::vec3 aabbMax) {
		return isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, 0, 0))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, 0, 0))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, aabbMax.y, 0))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, 0, aabbMax.z))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, aabbMax.y, 0))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, 0, aabbMax.z))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, aabbMax.y, aabbMax.z))
			&& isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z));
	}
    
    std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
    
    glm::vec3 sampleBoxUniform(glm::vec3 boxMin, glm::vec3 boxMax) {
        float x = uniformDist(randEngine) * (boxMax.x - boxMin.x);
        float y = uniformDist(randEngine) * (boxMax.y - boxMin.y);
        float z = uniformDist(randEngine) * (boxMax.z - boxMin.z);
        return glm::vec3(x, y, z);
    }
    
    glm::vec3 sampleSphereUniform(glm::vec3 center, float radius) {
		float theta = 2 * glm::pi<float>() * uniformDist(randEngine);
        float phi = glm::acos(1 - 2 * uniformDist(randEngine));
        float x = glm::sin(phi) * glm::cos(theta);
        float y = glm::sin(phi) * glm::cos(theta);
        float z = glm::cos(phi);
        return center + glm::vec3(x, y, z) * radius;
    }
}

ReflProbeBaker::ReflProbeBaker(RenderPipeline& pipeline, const PathTracer& pathTracer)
        : pipeline(&pipeline), pathTracer(&pathTracer) {
    // Do nothing
}

void ReflProbeBaker::generateEmptyProbeGrid(const Scene& scene, glm::ivec3 gridDim) {
	// Compute voxel grid
    scene.getBoundingBox(gridMin, gridMax);
    gridMin.y += 0.5f; // Make sure no probes are generated "on the ground"
    gridMax.y += 2.0f; // Make sure some reflections above the scene are captured
    
    this->voxelSize = (gridMax - gridMin) / glm::vec3(gridDim);
    this->gridDimensions = gridDim;
    
    // Determine voxel types
    voxelTypes.resize(gridDim.x * gridDim.y * gridDim.z, VoxelType::Empty);
    forEachVoxel([&](int x, int y, int z) {
        voxelTypes[getVoxelIndex({x, y, z})] = determineVoxelType(scene.getPrimitives(), getVoxelCenterWS({x, y, z}));
    });
    
    // Generate probe for empty voxel
	int layer = 0;
    forEachVoxel([&](int x, int y, int z) {
        int voxelIndex = getVoxelIndex({x, y, z});
        if (voxelTypes[voxelIndex] == VoxelType::Empty) {
            ReflectionProbe probe;
            probe.position = getVoxelCenterWS({x, y, z});
            //probe.aabbMin = probe.position - voxelSize * 0.5f; // TODO: Compute correct bounds
            //probe.aabbMax = probe.position + voxelSize * 0.5f; // TODO: Compute correct bounds
			probe.layer = layer;
			computeProbeAABB(probe.position, probe.aabbMin, probe.aabbMax);
            probes.push_back(probe);
			layer++;
        }
    });

	// Generate the visibility information for each surface voxel
	probeVisibility.resize(gridDim.x * gridDim.y * gridDim.z, glm::ivec3(-1));
	forEachVoxel([&](int x, int y, int z) {
		int voxelIndex = getVoxelIndex({ x, y, z });
		if (voxelTypes[voxelIndex] == VoxelType::Surface) {
			
			std::vector<int> probeVisibilityCounts(probes.size(), 0);

			int numSamples = 1024;
			for (int sample = 0; sample < numSamples; ++sample) {
				glm::vec3 samplePoint = getValidSampleInVoxel({ x, y, z }, scene);
				glm::vec3 sampleDir;
				glm::vec3 intersectionNormal;
				float tfar = -1.0f;
				while (tfar < 0.0f) {
					sampleDir = sampleSphereUniform(glm::vec3(0.0f), 1.0f);
					tfar = pathTracer->testIntersection(samplePoint, sampleDir, intersectionNormal);
				}
				glm::vec3 intersection = samplePoint + sampleDir * tfar;
				glm::vec3 reflectionVector = glm::reflect(sampleDir, intersectionNormal);

				for (std::size_t probeIndex = 0; probeIndex < probes.size(); ++probeIndex) {
					glm::vec3 toProbe = probes[probeIndex].position - intersection;
					float distToProbe = glm::length(toProbe);
					if (pathTracer->testOcclusionDist(intersection, toProbe) >= distToProbe) {
						probeVisibilityCounts[probeIndex]++;
					}
				}
			}

			std::vector<int> probeIndices(probes.size());
			std::iota(probeIndices.begin(), probeIndices.end(), 0);
			std::sort(probeIndices.begin(), probeIndices.end(), [&](int a, int b) {
				if (probeVisibilityCounts[a] == probeVisibilityCounts[b]) {
					return glm::distance2(probes[a].position, getVoxelCenterWS({ x, y, z }))
						< glm::distance2(probes[b].position, getVoxelCenterWS({ x, y, z }));
				}

				return probeVisibilityCounts[a] > probeVisibilityCounts[b];
			});

			glow::info() << probeIndices[0] << "  " << probeIndices[1] << "  " << probeIndices[2];

			probeVisibility[getVoxelIndex({ x, y, z })] = glm::ivec3(0);// glm::ivec3(probeIndices[0], probeIndices[1], probeIndices[2]);
		}
	});

	/*
	// Compute voxel grid
	scene.getBoundingBox(gridMin, gridMax);
	
	voxelSize = (gridMax - gridMin) / glm::vec3(gridDim);
	gridMin.x -= voxelSize.x / 4.0f;
	gridMin.y += 0.5f;
	gridMin.z -= voxelSize.z / 4.0f;
	gridMax += voxelSize / 4.0f;
	gridMax.y += 0.5f;
	voxelSize = (gridMax - gridMin) / glm::vec3(gridDim);
	gridDimensions = gridDim;

	// Generate env maps for empty and surface voxels
	int layer = 0;
	for (int z = 0; z <= gridDimensions.z; ++z) {
		for (int y = 0; y <= gridDimensions.y; ++y) {
			for (int x = 0; x <= gridDimensions.x; ++x) {
				ReflectionProbe probe;
				probe.position = gridMin + glm::vec3(x, y, z) * voxelSize;
				probe.aabbMin = probe.position - voxelSize * 0.5f; // TODO: Compute correct bounds
				probe.aabbMax = probe.position + voxelSize * 0.5f; // TODO: Compute correct bounds
				probe.layer = layer;
				probes.push_back(probe);
				layer++;
			}
		}
	}*/
}

std::vector<std::vector<unsigned int>> ReflProbeBaker::computePrimitiveProbeIndices(const Scene& scene) {
    std::vector<std::vector<unsigned int>> primitiveProbeIndices;
    
	/*
    // Find K closest probes vor each vertex
    for (const auto& prim : scene.getPrimitives()) {
        std::vector<glm::uvec4> probeIndices;
        for (auto vertex : prim.positions) {
            glm::vec3 vertexWS = prim.transform * glm::vec4(vertex, 1.0f);
            
            std::vector<int> indices(probes.size());
            std::iota(indices.begin(), indices.end(), 0);
            
            std::vector<float> distances(probes.size());
            for (std::size_t i = 0; i < probes.size(); ++i) {
                distances[i] = glm::distance2(probes[i].position, vertexWS);
            }
            
            std::sort(indices.begin(), indices.end(), [&](int a, int b) {
                return distances[a] < distances[b];
            });
            
            probeIndices.push_back({ indices[0], indices[1], indices[2], indices[4] });
        }
        
        primitiveProbeIndices.push_back(probeIndices);
    }*/

	for (const auto& prim : scene.getPrimitives()) {
		std::vector<unsigned int> probeIndices;
		for (auto vertex : prim.positions) {
			glm::vec3 vertexWS = prim.transform * glm::vec4(vertex, 1.0f);
			glm::ivec3 voxelCoord(-1);
			for (int z = 0; z < gridDimensions.z; ++z) {
				for (int y = 0; y < gridDimensions.y; ++y) {
					for (int x = 0; x < gridDimensions.x; ++x) {
						glm::vec3 voxelMin = gridMin + glm::vec3(x, y, z) * voxelSize;
						glm::vec3 voxelMax = voxelMin + voxelSize;

						if (isPointInsideBox(vertexWS, voxelMin, voxelMax)) {
							voxelCoord = { x, y, z };
							break;
						}
					}

					if (voxelCoord.x >= 0) {
						break;
					}
				}

				if (voxelCoord.x >= 0) {
					break;
				}
			}

			int probeIndex000 = getVoxelIndex(voxelCoord + glm::ivec3(0, 0, 0));
			int probeIndex100 = getVoxelIndex(voxelCoord + glm::ivec3(1, 0, 0));
			int probeIndex010 = getVoxelIndex(voxelCoord + glm::ivec3(0, 1, 0));
			int probeIndex001 = getVoxelIndex(voxelCoord + glm::ivec3(0, 0, 1));
			int probeIndex110 = getVoxelIndex(voxelCoord + glm::ivec3(1, 1, 0));
			int probeIndex101 = getVoxelIndex(voxelCoord + glm::ivec3(1, 0, 1));
			int probeIndex011 = getVoxelIndex(voxelCoord + glm::ivec3(0, 1, 1));
			int probeIndex111 = getVoxelIndex(voxelCoord + glm::ivec3(1, 1, 1));
			
			probeIndices.push_back(probes[probeIndex000].layer);
			probeIndices.push_back(probes[probeIndex100].layer);
			probeIndices.push_back(probes[probeIndex010].layer);
			probeIndices.push_back(probes[probeIndex001].layer);
			probeIndices.push_back(probes[probeIndex110].layer);
			probeIndices.push_back(probes[probeIndex101].layer);
			probeIndices.push_back(probes[probeIndex011].layer);
			probeIndices.push_back(probes[probeIndex111].layer);
		}

		primitiveProbeIndices.push_back(probeIndices);
	}
    
    return primitiveProbeIndices;
}

std::vector<ReflectionProbe>& ReflProbeBaker::getReflectionProbes() {
    return probes;
}

glm::vec3 ReflProbeBaker::getProbeVoxelSize() const {
	return voxelSize;
}

glm::ivec3 ReflProbeBaker::getProbeGridDimensions() const {
	return gridDimensions;
}

std::vector<glm::ivec3>& ReflProbeBaker::getProbeVisibility() {
	return probeVisibility;
}

ReflProbeBaker::VoxelType ReflProbeBaker::determineVoxelType(const std::vector<Primitive>& primitives, glm::vec3 voxelPos) const {
    glm::vec3 voxelMin = voxelPos;
    glm::vec3 voxelMax = voxelMin + voxelSize;
    
    VoxelType type = VoxelType::Empty;
	for (const auto& prim : primitives) {
		if (isBoxInsidePrimitve(prim, voxelMin, voxelMax)) {
			return VoxelType::Full;
		}
		
		// Check for half intersection
		for (std::size_t i = 0; i < prim.indices.size(); i += 3) {
			glm::vec3 A = prim.positions[prim.indices[i]];
			glm::vec3 B = prim.positions[prim.indices[i + 1]];
			glm::vec3 C = prim.positions[prim.indices[i + 2]];

			A = prim.transform * glm::vec4(A, 1.0f);
			B = prim.transform * glm::vec4(B, 1.0f);
			C = prim.transform * glm::vec4(C, 1.0f);

			if (testTriangleAABB(A, B, C, voxelMin, voxelMax)) {
				return VoxelType::Surface;
			}
			/*
			glm::vec3 v0 = B - A;
			glm::vec3 v1 = C - A;
			glm::vec3 N = glm::normalize(glm::cross(v0, v1));

			glm::vec3 triCenter = (v0 + v1) * 0.5f;
			glm::vec3 probePos = voxelPos + voxelSize * 0.5f;

			float distToCenter = glm::dot(N, probePos - triCenter);
			if (distToCenter < 0.0f) {
				return VoxelType::Full;
			}
			else {
				if (distToCenter < glm::max(voxelSize.x, glm::max(voxelSize.y, voxelSize.z)) * 1) {
					return VoxelType::Full;
				}
				else {
					type = VoxelType::Surface;
				}
			}*/
		}
	}
	


    return type;
}

glm::vec3 ReflProbeBaker::getVoxelCenterWS(glm::ivec3 voxelCoord) const {
    return gridMin + glm::vec3(voxelCoord) * voxelSize + voxelSize * 0.5f;
}

std::size_t ReflProbeBaker::getVoxelIndex(glm::ivec3 voxelCoord) const {
    return voxelCoord.x + voxelCoord.y * gridDimensions.x + voxelCoord.z * gridDimensions.x * gridDimensions.y;
}

void ReflProbeBaker::forEachVoxel(const std::function<void(int, int, int)>& body) {
    for (int z = 0; z < gridDimensions.z; ++z) {
        for (int y = 0; y < gridDimensions.y; ++y) {
            for (int x = 0; x < gridDimensions.x; ++x) {
                body(x, y, z);
            }
        } 
    }
}

glm::vec3 ReflProbeBaker::getValidSampleInVoxel(glm::ivec3 coord, const Scene& scene) const {
	glm::vec3 voxelMin = gridMin + glm::vec3(coord) * voxelSize;
	glm::vec3 voxelMax = voxelMin + voxelSize;

	bool isInMesh = true;
	glm::vec3 samplePoint;
	while (isInMesh) {
		isInMesh = false;
		samplePoint = sampleBoxUniform(voxelMin, voxelMax);
		for (const auto& prim : scene.getPrimitives()) {
			if (isPointInsidePrimitve(prim, samplePoint)) {
				isInMesh = true;
				break;
			}
		}
	}
	
	return samplePoint;
}

void ReflProbeBaker::computeProbeAABB(glm::vec3 probePos, glm::vec3& outMin, glm::vec3& outMax) const {
	float tfarLeft = pathTracer->testOcclusionDist(probePos, glm::vec3(-1, 0, 0));
	outMin.x = tfarLeft < 0.0f ? -std::numeric_limits<float>::max() : probePos.x - tfarLeft;

	float tfarRight = pathTracer->testOcclusionDist(probePos, glm::vec3(1, 0, 0));
	outMax.x = tfarRight < 0.0f ? std::numeric_limits<float>::max() : probePos.x + tfarRight;

	float tfarBottom = pathTracer->testOcclusionDist(probePos, glm::vec3(0, -1, 0));
	outMin.y = tfarBottom < 0.0f ? -std::numeric_limits<float>::max() : probePos.y - tfarBottom;

	float tfarTop = pathTracer->testOcclusionDist(probePos, glm::vec3(0, 1, 0));
	outMax.y = tfarTop < 0.0f ? std::numeric_limits<float>::max() : probePos.y + tfarTop;

	float tfarBack = pathTracer->testOcclusionDist(probePos, glm::vec3(0, 0, -1));
	outMin.z = tfarBack < 0.0f ? -std::numeric_limits<float>::max() : probePos.z - tfarBack;

	float tfarFront = pathTracer->testOcclusionDist(probePos, glm::vec3(0, 0, 1));
	outMax.z = tfarFront < 0.0f ? std::numeric_limits<float>::max() : probePos.z + tfarFront;
}
