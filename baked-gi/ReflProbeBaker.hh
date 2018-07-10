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
    
    void bake(const Scene& scene, glm::ivec3 gridDim, int envMapRes = 128);
    std::vector<ReflectionProbe>& getReflectionProbes();
    
private:
    enum class VoxelType {
        Empty,
        Surface,
        Full
    };
    
    VoxelType determineVoxelType(const std::vector<Primitive>& primitives, glm::vec3 voxelPos) const;
    
    RenderPipeline* pipeline;
    const PathTracer* pathTracer;

    glm::ivec3 gridDimensions;
    glm::vec3 voxelSize;
    std::vector<VoxelType> voxelTypes;
    std::vector<ReflectionProbe> probes;
    
    using ProbeIndices = std::vector<int>;
    using VertexProbeTable = std::vector<ProbeIndices>;
    std::vector<VertexProbeTable> meshProbeTable;
};
