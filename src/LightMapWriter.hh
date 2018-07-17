#pragma once

#include "Image.hh"

#include <string>
#include <vector>

void writeLightMapToFile(const std::string& path, const std::vector<SharedImage>& irradianceMaps);
void writeLightMapToFile(const std::string& path, const std::vector<SharedImage>& irradianceMaps, const std::vector<SharedImage>& aoMaps);