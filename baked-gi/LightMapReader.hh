#pragma once

#include "Image.hh"

#include <string>
#include <vector>

void readLightMapFromFile(const std::string& path, std::vector<SharedImage>& irradianceMaps);
void readLightMapFromFile(const std::string& path, std::vector<SharedImage>& irradianceMaps, std::vector<SharedImage>& aoMaps);