#pragma once

#include "ReflectionProbe.hh"
#include "Primitive.hh"

#include <glm/glm.hpp>
#include <vector>

class RenderPipeline;
class PathTracer;
class Scene;

class ReflProbeBaker {
public:
    ReflProbeBaker(RenderPipeline& pipeline, const PathTracer& pathTracer);
    
    void generateEmptyProbeGrid(const Scene& scene, glm::ivec3 gridDim);
    std::vector<std::vector<glm::uvec4>> computePrimitiveProbeIndices(const Scene& scene);
    const std::vector<ReflectionProbe>& bakeGGXEnvProbes(const Scene& scene, int envMapRes);
    std::vector<ReflectionProbe>& getReflectionProbes();
    
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
    
    RenderPipeline* pipeline;
    const PathTracer* pathTracer;

    glm::ivec3 gridDimensions;
    glm::vec3 gridMin;
    glm::vec3 gridMax;
    glm::vec3 voxelSize;
    std::vector<VoxelType> voxelTypes;
    std::vector<ReflectionProbe> probes;
};
