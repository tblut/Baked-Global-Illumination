#pragma once

#include "ReflectionProbe.hh"
#include "VoxelGrid.hh"

#include <glm/glm.hpp>
#include <string>
#include <vector>

void writeProbeDataToFile(const std::string& path, const std::vector<ReflectionProbe>& probes, const VoxelGrid<glm::ivec3>& visibilityGrid);