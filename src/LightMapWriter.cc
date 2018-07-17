#include "LightMapWriter.hh"

#include <glm/glm.hpp>
#include <fstream>
#include <cstdint>

void writeLightMapToFile(const std::string& path, const std::vector<SharedImage>& irradianceMaps) {
	writeLightMapToFile(path, irradianceMaps, std::vector<SharedImage>());
}

void writeLightMapToFile(const std::string& path, const std::vector<SharedImage>& irradianceMaps, const std::vector<SharedImage>& aoMaps) {
	std::ofstream outputFile(path, std::ios::binary | std::ios::trunc | std::ios::out);

	std::uint32_t numIrradianceMaps = static_cast<std::uint32_t>(irradianceMaps.size());
	outputFile.write(reinterpret_cast<const char*>(&numIrradianceMaps), sizeof(std::uint32_t));

	std::uint32_t numAoMaps = static_cast<std::uint32_t>(aoMaps.size());
	outputFile.write(reinterpret_cast<const char*>(&numAoMaps), sizeof(std::uint32_t));

	for (const auto& map : irradianceMaps) {
		std::uint32_t width = map->getWidth();
		std::uint32_t height = map->getHeight();
		outputFile.write(reinterpret_cast<const char*>(&width), sizeof(std::uint32_t));
		outputFile.write(reinterpret_cast<const char*>(&height), sizeof(std::uint32_t));
		outputFile.write(map->getDataPtr<char>(), width * height * sizeof(glm::u16vec3));
	}

	for (const auto& map : aoMaps) {
		std::uint32_t width = map->getWidth();
		std::uint32_t height = map->getHeight();
		outputFile.write(reinterpret_cast<const char*>(&width), sizeof(std::uint32_t));
		outputFile.write(reinterpret_cast<const char*>(&height), sizeof(std::uint32_t));
		outputFile.write(map->getDataPtr<char>(), width * height * sizeof(glm::uint16));
	}

	outputFile.close();
}