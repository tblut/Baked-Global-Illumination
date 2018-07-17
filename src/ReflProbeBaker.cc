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

	// See: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	bool testRayTriangle(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 triA, glm::vec3 triB, glm::vec3 triC) {
		const float EPSILON = 0.0000001f;
		auto edge1 = triB - triA;
		auto edge2 = triC - triA;
		auto h = glm::cross(rayDir, edge2);
		auto a = glm::dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) {
			return false;
		}
		auto f = 1.0f / a;
		auto s = rayOrigin - triA;
		auto u = f * glm::dot(s, h);
		if (u < 0.0f || u > 1.0f) {
			return false;
		}
		auto q = glm::cross(s, edge1);
		auto v = f * glm::dot(rayDir, q);
		if (v < 0.0 || u + v > 1.0) {
			return false;
		}
		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * glm::dot(edge2, q);
		return t > EPSILON;
	}

	bool isPointInsidePrimitve(const Primitive& primitive, glm::vec3 point) {
		int intersectionCount = 0;
		for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
			glm::vec3 A = primitive.positions[primitive.indices[i]];
			glm::vec3 B = primitive.positions[primitive.indices[i + 1]];
			glm::vec3 C = primitive.positions[primitive.indices[i + 2]];

			A = primitive.transform * glm::vec4(A, 1.0f);
			B = primitive.transform * glm::vec4(B, 1.0f);
			C = primitive.transform * glm::vec4(C, 1.0f);

			if (testRayTriangle(point, glm::vec3(1.0f, 0.0f, 0.0f), A, B, C)) {
				intersectionCount++;
			}
		}

		return intersectionCount % 2 == 1;
	}

	bool isBoxInsidePrimitve(const Primitive& primitive, glm::vec3 aabbMin, glm::vec3 aabbMax) {
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, 0, 0))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, 0, 0))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, aabbMax.y, 0))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, 0, aabbMax.z))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, aabbMax.y, 0))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, 0, aabbMax.z))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(0, aabbMax.y, aabbMax.z))) return false;
		if (!isPointInsidePrimitve(primitive, aabbMin + glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z))) return false;
		return true;
	}

	float signedDistancePointTriangle(glm::vec3 point, glm::vec3 triA, glm::vec3 triB, glm::vec3 triC) {
		auto triNormal = glm::normalize(glm::cross(triB - triA, triC - triA));
		return glm::dot(triNormal, point - triA);
	}
	
	glm::vec3 projectPointOntoPlane(glm::vec3 point, glm::vec3 A, glm::vec3 B, glm::vec3 C) {
        glm::vec3 n = glm::normalize(glm::cross(B - A, C - A));
        return point - glm::dot(n, point - A) * n;
    }

    std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);
    
    glm::vec3 sampleBoxUniform(glm::vec3 boxMin, glm::vec3 boxMax) {
        float x = uniformDist(randEngine) * (boxMax.x - boxMin.x);
        float y = uniformDist(randEngine) * (boxMax.y - boxMin.y);
        float z = uniformDist(randEngine) * (boxMax.z - boxMin.z);
        return boxMin + glm::vec3(x, y, z);
    }
    
    glm::vec3 sampleSphereUniform(glm::vec3 center, float radius) {
		float theta = 2 * glm::pi<float>() * uniformDist(randEngine);
        float phi = glm::acos(1 - 2 * uniformDist(randEngine));
        float x = glm::sin(phi) * glm::cos(theta);
        float y = glm::sin(phi) * glm::cos(theta);
        float z = glm::cos(phi);
        return center + glm::vec3(x, y, z) * radius;
    }

	glm::vec3 sampleTriangleUniform(glm::vec3 triA, glm::vec3 triB, glm::vec3 triC) {
		float u = uniformDist(randEngine);
		float v = uniformDist(randEngine);

		if (u + v > 1.0f) {
			u = 1.0f - u;
			v = 1.0f - v;
		}

		return u * triA + v * triB + (1.0f - u - v) * triC;
	}

	void makeCoordinateSystem(const glm::vec3& normal, glm::vec3& xAxis, glm::vec3& yAxis) {
		xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		if (std::abs(1.0f - normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, -1.0f);
		}
		else if (std::abs(1.0f + normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		yAxis = glm::normalize(glm::cross(normal, xAxis));
		xAxis = glm::normalize(glm::cross(yAxis, normal));
	}

	glm::vec3 sampleCosineHemisphere(const glm::vec3& normal) {
		float u1 = uniformDist(randEngine);
		float u2 = uniformDist(randEngine);

		float r = std::sqrt(u1);
		float phi = 2.0f * glm::pi<float>() * u2;

		float x = r * std::sin(phi);
		float y = r * std::cos(phi);
		float z = std::sqrt(1.0f - x * x - y * y);

		glm::vec3 xAxis;
		glm::vec3 yAxis;
		makeCoordinateSystem(normal, xAxis, yAxis);

		return glm::normalize(x * xAxis + y * yAxis + z * normal);
	}
}

ReflProbeBaker::ReflProbeBaker(RenderPipeline& pipeline, const PathTracer& pathTracer)
        : pipeline(&pipeline), pathTracer(&pathTracer) {
    // Do nothing
}

void ReflProbeBaker::generateEmptyProbeGrid(const Scene& scene, glm::ivec3 gridDim) {
	// Compute voxel grid
	glm::vec3 gridMin, gridMax;
    scene.getBoundingBox(gridMin, gridMax);
	gridMin -= glm::vec3(0.1f);
	gridMax -= glm::vec3(0.1f);
    //gridMin.y += 0.5f; // Make sure no probes are generated "on the ground"
    gridMax.y += 2.0f; // Make sure some reflections above the scene are captured

	// Determine which fine grained voxels contain a triangle.
	// The ones that do are considered solid. This information
	// is then used later on to determine where in the more coarse grid
	// probes should be placed.
	fineVoxelGrid.reset(new VoxelGrid<VoxelType>(gridMin, gridMax, gridDim * fineGridScale));
	fineVoxelGrid->fill(VoxelType::Empty);
	fineVoxelGrid->forEachVoxel([&](int x, int y, int z) {
		auto type = determineFineVoxelType(scene.getPrimitives(), { x, y, z });
		fineVoxelGrid->setVoxel({ x, y, z }, type);
	});

    // Based on the previous step determine the type of each coarse voxel
	coarseVoxelGrid.reset(new VoxelGrid<VoxelType>(gridMin, gridMax, gridDim));
	coarseVoxelGrid->fill(VoxelType::Empty);
	coarseVoxelGrid->forEachVoxel([&](int x, int y, int z) {
		auto type = determineCoarseVoxelType(scene.getPrimitives(), { x, y, z });
		coarseVoxelGrid->setVoxel({ x, y, z }, type);
	});
    
    // Generate probe for empty voxel
	int layer = 0;
	coarseVoxelGrid->forEachVoxel([&](int x, int y, int z) {
		if (coarseVoxelGrid->getVoxel({ x, y, z }) != VoxelType::Solid) {
			ReflectionProbe probe;
			probe.position = coarseVoxelGrid->getVoxelCenter({ x, y, z });
			probe.layer = layer;
			computeProbeAABB(probe.position, probe.aabbMin, probe.aabbMax);
			probes.push_back(probe);
			layer++;
		}
	});
    
    // Compute visibility grid
	glm::ivec3 visGridDim = gridDim * fineGridScale;
	probeVisibilityGrid.reset(new VoxelGrid<glm::ivec3>(gridMin, gridMax, visGridDim));
	probeVisibilityGrid->fill(glm::ivec3(-1));
    
    // Determine which triangles are in which triangle
    voxelTriangles.resize(visGridDim.x * visGridDim.y * visGridDim.z);
    probeVisibilityGrid->forEachVoxel([&](int x, int y, int z) {
        glm::vec3 voxelMin = probeVisibilityGrid->getVoxelMin({ x, y, z });
        glm::vec3 voxelMax = probeVisibilityGrid->getVoxelMax({ x, y, z });
        int voxelIndex = probeVisibilityGrid->getVoxelIndex({ x, y, z });
        
        for (const auto& prim : scene.getPrimitives()) {
            for (std::size_t i = 0; i < prim.indices.size(); i += 3) {
                glm::vec3 A = prim.positions[prim.indices[i]];
                glm::vec3 B = prim.positions[prim.indices[i + 1]];
                glm::vec3 C = prim.positions[prim.indices[i + 2]];

                A = prim.transform * glm::vec4(A, 1.0f);
                B = prim.transform * glm::vec4(B, 1.0f);
                C = prim.transform * glm::vec4(C, 1.0f);

                if (testTriangleAABB(A, B, C, voxelMin, voxelMax)) {
                    voxelTriangles[voxelIndex].push_back(A);
                    voxelTriangles[voxelIndex].push_back(B);
                    voxelTriangles[voxelIndex].push_back(C);
                }
            }
        }
        
    });

	

	// Generate the visibility information for each surface voxel
	/*probeVisibilityGrid->forEachVoxel([&](int x, int y, int z) {
		glm::ivec3 typeCoord = glm::ivec3(x, y, z) / fineGridScale;
		if (coarseVoxelGrid->getVoxel(typeCoord) != VoxelType::Empty) {
			std::vector<int> probeVisibilityCounts(probes.size(), 0);

			int numSamples = 1024;
			for (int sample = 0; sample < numSamples; ++sample) {
				glm::vec3 samplePoint, triangleNormal;
				getValidSampleInVoxel({ x, y, z }, scene, samplePoint, triangleNormal);
				
				glm::vec3 reflectionDir = sampleCosineHemisphere(triangleNormal);
				glm::vec3 intersectionNormal;
				float tfar = pathTracer->testIntersection(samplePoint, reflectionDir, intersectionNormal);
				if (tfar < 0.0f) {
					for (std::size_t probeIndex = 0; probeIndex < probes.size(); ++probeIndex) {
						// Test if the probe can see the same direction
						//if (pathTracer->testOcclusionDist(probes[probeIndex].position, reflectionDir) < 0.0f) {
						//	probeVisibilityCounts[probeIndex]++;
						//}
					}
				}
				else {
					glm::vec3 intersection = samplePoint + reflectionDir * tfar;
					glm::vec3 reflectionVector = glm::reflect(reflectionDir, intersectionNormal);

					for (std::size_t probeIndex = 0; probeIndex < probes.size(); ++probeIndex) {
						glm::vec3 toProbe = probes[probeIndex].position - intersection;
						float distToProbe = glm::length(toProbe);
						if (pathTracer->testOcclusionDist(intersection, toProbe) >= distToProbe) {
							probeVisibilityCounts[probeIndex]++;
						}
					}
				}
			}

			std::vector<int> probeIndices(probes.size());
			std::iota(probeIndices.begin(), probeIndices.end(), 0);
			std::sort(probeIndices.begin(), probeIndices.end(), [&](int a, int b) {
				if (probeVisibilityCounts[a] == probeVisibilityCounts[b]) {
					return glm::distance2(probes[a].position, probeVisibilityGrid->getVoxelCenter({ x, y, z }))
						< glm::distance2(probes[b].position, probeVisibilityGrid->getVoxelCenter({ x, y, z }));
				}

				return probeVisibilityCounts[a] > probeVisibilityCounts[b];
			});

			glow::info() << probeIndices[0] << "  " << probeIndices[1] << "  " << probeIndices[2];
			
			probeVisibilityGrid->setVoxel({ x, y, z }, glm::ivec3(probeIndices[0], probeIndices[1], probeIndices[2]));
		}
	});*/
    
    probeVisibilityGrid->forEachVoxel([&](int x, int y, int z) {
        glm::vec3 voxelCenter = probeVisibilityGrid->getVoxelCenter({ x, y, z });
        glm::vec3 samplePoint = voxelCenter;
        
        int voxelIndex = probeVisibilityGrid->getVoxelIndex({ x, y, z });
        if (!voxelTriangles[voxelIndex].empty()) {
            glm::vec3 voxelCenter = probeVisibilityGrid->getVoxelCenter({ x, y, z });
            samplePoint = projectPointOntoPlane(voxelCenter, voxelTriangles[voxelIndex][0],
                voxelTriangles[voxelIndex][1], voxelTriangles[voxelIndex][2]);
        }
        
        std::vector<int> visibleProbes;
        for (int probeIndex = 0; probeIndex < probes.size(); ++probeIndex) {
            //glm::vec3 toProbe = glm::normalize(probes[probeIndex].position - samplePoint);
            //if (pathTracer->testOcclusionDist(samplePoint, toProbe) > 0.0f) {
                visibleProbes.push_back(probeIndex);
            //} 
        }
        
        std::sort(visibleProbes.begin(), visibleProbes.end(), [&](int a, int b) {
            glm::vec3 posA = probes[a].position;
            glm::vec3 posB = probes[b].position;
			return glm::distance2(posA, voxelCenter) < glm::distance2(posB, voxelCenter);
        });
        
        if (visibleProbes.size() >= 3) {
            int layer0 = probes[visibleProbes[0]].layer;
            int layer1 = probes[visibleProbes[1]].layer;
            int layer2 = probes[visibleProbes[2]].layer;
            
            probeVisibilityGrid->setVoxel({ x, y, z }, glm::ivec3(layer0, layer1, layer2));
        }
        else {
            
        }
    });
}

const std::vector<ReflectionProbe>& ReflProbeBaker::getReflectionProbes() const {
    return probes;
}

const VoxelGrid<glm::ivec3>& ReflProbeBaker::getProbeVisibilityGrid() const {
	return *probeVisibilityGrid;
}

int ReflProbeBaker::getVisibilityGridScale() const {
	return fineGridScale;
}

ReflProbeBaker::VoxelType ReflProbeBaker::determineFineVoxelType(const std::vector<Primitive>& primitives, glm::ivec3 coord) const {
    glm::vec3 voxelMin = fineVoxelGrid->getVoxelMin(coord);
    glm::vec3 voxelMax = fineVoxelGrid->getVoxelMax(coord);
	glm::vec3 voxelCenter = fineVoxelGrid->getVoxelCenter(coord);
    
    VoxelType type = VoxelType::Empty;
	for (const auto& prim : primitives) {
		if (isBoxInsidePrimitve(prim, voxelMin, voxelMax)) {
			return VoxelType::Solid;
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
		}
	}
	
    return type;
}

ReflProbeBaker::VoxelType ReflProbeBaker::determineCoarseVoxelType(const std::vector<Primitive>& primitives, glm::ivec3 coarseCoord) const {
	glm::ivec3 fineVoxelCoordStart = coarseCoord * fineGridScale;
	glm::ivec3 fineVoxelCoordEnd = fineVoxelCoordStart + glm::ivec3(fineGridScale);

	int solidCount = 0;
	for (int z = fineVoxelCoordStart.z; z < fineVoxelCoordEnd.z; ++z) {
		for (int y = fineVoxelCoordStart.y; y < fineVoxelCoordEnd.y; ++y) {
			for (int x = fineVoxelCoordStart.x; x < fineVoxelCoordEnd.x; ++x) {
				if (fineVoxelGrid->getVoxel({ x, y, z }) != VoxelType::Empty) {
					solidCount++;
				}
			}
		}
	}

	VoxelType type;
	int voxelArea = fineGridScale * fineGridScale * fineGridScale;
	if (solidCount == 0) {
		type = VoxelType::Empty;
	}
	else if (solidCount < voxelArea / 4) {
		type = VoxelType::Surface;
	}
	else {
		type = VoxelType::Solid;
	}
	
	return type;
}

void ReflProbeBaker::getValidSampleInVoxel(glm::ivec3 coord, const Scene& scene, glm::vec3& samplePoint, glm::vec3& triNormal) const {
	glm::vec3 voxelMin = probeVisibilityGrid->getVoxelMin(coord);
	glm::vec3 voxelMax = probeVisibilityGrid->getVoxelMax(coord);
	int voxelIndex = probeVisibilityGrid->getVoxelIndex(coord);

    glm::vec3 point = sampleBoxUniform(voxelMin, voxelMax);
    if (voxelTriangles[voxelIndex].size() == 0) {
        samplePoint = point;
        triNormal = sampleSphereUniform(glm::vec3(0.0f), 1.0f);
        return;
    }
    
    int triangleIndex = static_cast<int>(uniformDist(randEngine) * voxelTriangles[voxelIndex].size() / 3);
    glow::info() << triangleIndex << "/" << voxelTriangles[voxelIndex].size() / 3;
    
    glm::vec3 triA = voxelTriangles[voxelIndex][triangleIndex * 3];
    glm::vec3 triB = voxelTriangles[voxelIndex][triangleIndex * 3 + 1];
    glm::vec3 triC = voxelTriangles[voxelIndex][triangleIndex * 3 + 2];
    samplePoint = projectPointOntoPlane(point, triA, triB, triC);
    triNormal = glm::normalize(glm::cross(triB - triA, triC - triA));
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
