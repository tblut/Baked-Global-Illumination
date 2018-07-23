#include "ProbeDataWriter.hh"
#include <fstream>

void writeProbeDataToFile(const std::string& path, const std::vector<ReflectionProbe>& probes,
		int textureSize, int numBounces, const VoxelGrid<glm::ivec3>& visibilityGrid) {
	std::ofstream outputFile(path, std::ios::binary | std::ios::trunc | std::ios::out);

	std::uint32_t numProbes = static_cast<std::uint32_t>(probes.size());
	outputFile.write(reinterpret_cast<const char*>(&numProbes), sizeof(std::uint32_t));
	outputFile.write(reinterpret_cast<const char*>(&textureSize), sizeof(std::int32_t));
	outputFile.write(reinterpret_cast<const char*>(&numBounces), sizeof(std::int32_t));

	for (const auto& probe : probes) {
		outputFile.write(reinterpret_cast<const char*>(&probe.position.x), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.position.y), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.position.z), sizeof(float));

		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMin.x), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMin.y), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMin.z), sizeof(float));

		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMax.x), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMax.y), sizeof(float));
		outputFile.write(reinterpret_cast<const char*>(&probe.aabbMax.z), sizeof(float));

		outputFile.write(reinterpret_cast<const char*>(&probe.layer), sizeof(std::uint32_t));
	}

	glm::ivec3 dim = visibilityGrid.getDimensions();
	outputFile.write(reinterpret_cast<const char*>(&dim.x), sizeof(std::int32_t));
	outputFile.write(reinterpret_cast<const char*>(&dim.y), sizeof(std::int32_t));
	outputFile.write(reinterpret_cast<const char*>(&dim.z), sizeof(std::int32_t));

	glm::vec3 min = visibilityGrid.getMin();
	outputFile.write(reinterpret_cast<const char*>(&min.x), sizeof(float));
	outputFile.write(reinterpret_cast<const char*>(&min.y), sizeof(float));
	outputFile.write(reinterpret_cast<const char*>(&min.z), sizeof(float));

	glm::vec3 max = visibilityGrid.getMax();
	outputFile.write(reinterpret_cast<const char*>(&max.x), sizeof(float));
	outputFile.write(reinterpret_cast<const char*>(&max.y), sizeof(float));
	outputFile.write(reinterpret_cast<const char*>(&max.z), sizeof(float));

	outputFile.write(reinterpret_cast<const char*>(visibilityGrid.getInternalArray().data()), sizeof(glm::ivec3) * dim.x * dim.y * dim.z);

	outputFile.close();
}