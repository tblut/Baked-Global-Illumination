#include "LightMapReader.hh"

#include <glm/glm.hpp>
#include <fstream>
#include <cstdint>

void readLightMapFromFile(const std::string& path, std::vector<SharedImage>& irradianceMaps) {
	std::vector<SharedImage> temp;
	readLightMapFromFile(path, irradianceMaps, temp);
}

void readLightMapFromFile(const std::string& path, std::vector<SharedImage>& irradianceMaps, std::vector<SharedImage>& aoMaps) {
	std::ifstream inputFile(path, std::ios::binary | std::ios::in);

	std::uint32_t numIrradianceMaps;
	inputFile.read(reinterpret_cast<char*>(&numIrradianceMaps), sizeof(std::uint32_t));
	irradianceMaps.reserve(numIrradianceMaps);

	std::uint32_t numAoMaps;
	inputFile.read(reinterpret_cast<char*>(&numAoMaps), sizeof(std::uint32_t));
	aoMaps.reserve(numAoMaps);

	for (std::uint32_t i = 0; i < numIrradianceMaps; ++i) {
		std::uint32_t width;
		inputFile.read(reinterpret_cast<char*>(&width), sizeof(std::uint32_t));

		std::uint32_t height;
		inputFile.read(reinterpret_cast<char*>(&height), sizeof(std::uint32_t));

		SharedImage map = std::make_shared<Image>(width, height, GL_RGB16F);
		inputFile.read(map->getDataPtr<char>(), width * height * sizeof(glm::u16vec3));

		irradianceMaps.push_back(map);
	}

	for (std::uint32_t i = 0; i < numAoMaps; ++i) {
		std::uint32_t width;
		inputFile.read(reinterpret_cast<char*>(&width), sizeof(std::uint32_t));

		std::uint32_t height;
		inputFile.read(reinterpret_cast<char*>(&height), sizeof(std::uint32_t));

		SharedImage map = std::make_shared<Image>(width, height, GL_R16F);
		inputFile.read(map->getDataPtr<char>(), width * height * sizeof(glm::uint16));

		aoMaps.push_back(map);
	}
}