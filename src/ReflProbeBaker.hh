#pragma once

#include "ReflectionProbe.hh"
#include "Primitive.hh"
#include "VoxelGrid.hh"

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <memory>

class RenderPipeline;
class PathTracer;
class Scene;

class ReflProbeBaker {
public:
    ReflProbeBaker(RenderPipeline& pipeline, const PathTracer& pathTracer);
    
    void generateEmptyProbeGrid(const Scene& scene, glm::ivec3 gridDim);
    const std::vector<ReflectionProbe>& getReflectionProbes() const;
	const VoxelGrid<glm::ivec3>& getProbeVisibilityGrid() const;
	int getVisibilityGridScale() const;
    
private:
    enum class VoxelType {
        Empty,   // No geometry
		Surface, // Is less than half contained by geometry
		Solid    // Is fully contained by geometry
    };
    
    VoxelType determineFineVoxelType(const std::vector<Primitive>& primitives, glm::ivec3 coord) const;
	VoxelType determineCoarseVoxelType(const std::vector<Primitive>& primitives, glm::ivec3 coarseCoord) const;
	void getValidSampleInVoxel(glm::ivec3 coord, const Scene& scene, glm::vec3& samplePoint, glm::vec3& triNormal) const;
	void computeProbeAABB(glm::vec3 probePos, glm::vec3& outMin, glm::vec3& outMax) const;
    
    RenderPipeline* pipeline;
    const PathTracer* pathTracer;

    std::vector<ReflectionProbe> probes;
	std::unique_ptr<VoxelGrid<VoxelType>> fineVoxelGrid;
	std::unique_ptr<VoxelGrid<VoxelType>> coarseVoxelGrid;
	std::unique_ptr<VoxelGrid<glm::ivec3>> probeVisibilityGrid;
	std::vector<std::vector<glm::vec3>> voxelTriangles; // A list of triangles for each voxel

	const int fineGridScale = 4;
};
