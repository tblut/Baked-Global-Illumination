#include "ReflProbeBaker.hh"
#include "RenderPipeline.hh"
#include "PathTracer.hh"
#include "Scene.hh"

#include <glow/common/log.hh>
#include <limits>
#include <random>

/*
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
    
void ReflProbeBaker::bake(const Scene& scene, glm::ivec3 gridDim, int envMapRes) {
    // Compute voxel grid
    glm::vec3 min, max;
    scene.getBoundingBox(min, max);
  
    min.y += 1.0f;
    max.y += 1.0f;
    voxelSize = (max - min) / glm::vec3(gridDim.x, gridDim.y, gridDim.z);
    
    min += voxelSize * 0.2f;
    max -= voxelSize * 0.2f;
    voxelSize = (max - min) / glm::vec3(gridDim.x, gridDim.y, gridDim.z);
    gridDimensions = gridDim;
    
    // Determine voxel types
    voxelTypes.resize((gridDim.x + 1) * (gridDim.y + 1) * (gridDim.z + 1), VoxelType::Empty);
    for (int z = 0; z <= gridDim.z; ++z) {
        for (int y = 0; y <= gridDim.y; ++y) {
            for (int x = 0; x <= gridDim.x; ++x) {
                int voxelIndex = x + y * gridDim.x + z * gridDim.x * gridDim.y;
                glm::vec3 voxelPos = min + voxelSize * glm::vec3(x, y, z);
                voxelTypes[voxelIndex] = determineVoxelType(scene.getPrimitives(), voxelPos);
            }
        } 
    }
    
    // Generate env maps for empty and surface voxels
    for (int z = 0; z <= gridDim.z; ++z) {
        for (int y = 0; y <= gridDim.y; ++y) {
            for (int x = 0; x <= gridDim.x; ++x) {
                int voxelIndex = x + y * gridDim.x + z * gridDim.x * gridDim.y;
                if (voxelTypes[voxelIndex] == VoxelType::Full) {
                    continue;
                }
                
                ReflectionProbe probe;
                probe.position = min + voxelSize * glm::vec3(x, y, z);
                probe.aabbMin = probe.position - voxelSize * 0.5f; // TODO: Compute correct bounds
                probe.aabbMax = probe.position + voxelSize * 0.5f; // TODO: Compute correct bounds
                probe.ggxEnvMap = pipeline->renderEnvironmentMap(probe.position, envMapRes, scene.getMeshes());
                probes.push_back(probe);
            }
        } 
    }
    
    // Assign K best probes to all voxels
    for (int z = 0; z <= gridDim.z; ++z) {
        for (int y = 0; y <= gridDim.y; ++y) {
            for (int x = 0; x <= gridDim.x; ++x) {
                
                std::vector<int> probeViewCount(probes.size(), 0);
                
                int numSamples = 512;
                for (int sample = 0; sample < numSamples; ++sample) {
                    glm::vec3 voxelPos = min + voxelSize * glm::vec3(x, y, z);
                    glm::vec3 voxelMin = voxelPos - voxelSize * 0.5f;
                    glm::vec3 voxelMax = voxelPos + voxelSize * 0.5f;
                    glm::vec3 origin = sampleBoxUniform(voxelMin, voxelMax);
                    glm::vec3 dir = sampleSphereUniform(origin, 1.0f);
                    
                    glm::vec3 intersection;
                    if (pathTracer->testIntersection(origin, dir, intersection)) {
                        for (std::size_t i = 0; i < probes.size(); ++i) {
                            glm::vec3 toProbe = glm::normalize(probes[i].position - intersection);
                            if (pathTracer->testOcclusionDist(intersection, toProbe) < 0) {
                                probeViewCount[i]++;
                            }
                        }
                    }
                    
                }
            }
        } 
    }
}
    
std::vector<ReflectionProbe>& ReflProbeBaker::getReflectionProbes() {
    return probes;
}

ReflProbeBaker::VoxelType ReflProbeBaker::determineVoxelType(const std::vector<Primitive>& primitives, glm::vec3 voxelPos) const {
    glm::vec3 voxelMin = voxelPos;
    glm::vec3 voxelMax = voxelMin + voxelSize;
    
    VoxelType type = VoxelType::Empty;
    /*
    bool hasVertexInside = false;
    float minDistance = std::numeric_limits<float>::max();
    for (const auto& prim : primitives) {
        for (std::size_t i = 0; i < prim.indices.size(); i += 3) {
            glm::vec3 A = prim.positions[prim.indices[i]];
            glm::vec3 B = prim.positions[prim.indices[i + 1]];
            glm::vec3 C = prim.positions[prim.indices[i + 2]];
            
            A = prim.transform * glm::vec4(A, 1.0f);
            B = prim.transform * glm::vec4(B, 1.0f);
            C = prim.transform * glm::vec4(C, 1.0f);
            
            glm::vec3 v0 = B - A;
            glm::vec3 v1 = C - A;
            glm::vec3 N = glm::normalize(glm::cross(v0, v1));
            
            glm::vec3 triCenter = (v0 + v1) * 0.5f;
            glm::vec3 probePos = voxelPos;
            
            float distToCenter = glm::dot(N, probePos - triCenter);
            if (glm::abs(distToCenter) < glm::max(voxelSize.x, glm::max(voxelSize.y, voxelSize.z)) * 0.5f) {
                glow::info() << distToCenter;
                glow::info() << glm::length(voxelSize) * 0.5f;
                type = (distToCenter > 0.0f) ? VoxelType::Surface : VoxelType::Full;
                if (type == VoxelType::Full) {
                    return type;
                }
            }
        }
    }
*/
    return type;
}
