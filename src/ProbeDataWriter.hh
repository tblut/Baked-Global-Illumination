#pragma once

#include "ReflectionProbe.hh"
#include "VoxelGrid.hh"

#include <glm/glm.hpp>
#include <string>
#include <vector>

void writeProbeDataToFile(const std::string& path, const std::vector<ReflectionProbe>& probes,
	int textureSize, int numBounces, const VoxelGrid<glm::ivec3>& visibilityGrid);