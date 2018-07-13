#pragma once

#include "ReflectionProbe.hh"
#include "Primitive.hh"

#include <glm/glm.hpp>
#include <vector>
#include <functional>

class RenderPipeline;
class PathTracer;
class Scene;

class ReflProbeBaker {
public:
    ReflProbeBaker(RenderPipeline& pipeline, const PathTracer& pathTracer);
    
    void generateEmptyProbeGrid(const Scene& scene, glm::ivec3 gridDim);
    std::vector<std::vector<unsigned int>> computePrimitiveProbeIndices(const Scene& scene);
    std::vector<ReflectionProbe>& getReflectionProbes();
	glm::vec3 getProbeVoxelSize() const;
	glm::ivec3 getProbeGridDimensions() const;
	std::vector<glm::ivec3>& getProbeVisibility();
    
private:
    enum class VoxelType {
        Empty,
        Surface,
        Full
    };
    
    VoxelType determineVoxelType(const std::vector<Primitive>& primitives, glm::vec3 voxelPos) const;
    glm::vec3 getVoxelCenterWS(glm::ivec3 voxelCoord) const;
    std::size_t getVoxelIndex(glm::ivec3 voxelCoord) const;
    void forEachVoxel(const std::function<void(int, int, int)>& body);
	glm::vec3 getValidSampleInVoxel(glm::ivec3 coord, const Scene& scene) const;
	void computeProbeAABB(glm::vec3 probePos, glm::vec3& outMin, glm::vec3& outMax) const;
    
    RenderPipeline* pipeline;
    const PathTracer* pathTracer;

    glm::ivec3 gridDimensions;
    glm::vec3 gridMin;
    glm::vec3 gridMax;
    glm::vec3 voxelSize;
    std::vector<VoxelType> voxelTypes;
    std::vector<ReflectionProbe> probes;
	glow::SharedTextureCubeMapArray envMapArray;
	std::vector<glm::ivec3> probeVisibility;
};
