#pragma once

#include "ReflectionProbe.hh"
#include "VoxelGrid.hh"

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

std::shared_ptr<VoxelGrid<glm::ivec3>> readProbeDataToFile(const std::string& path, std::vector<ReflectionProbe>& outProbes);