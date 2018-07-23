#include "ProbeDataReader.hh"
#include <fstream>

#include <glow/common/log.hh>

std::shared_ptr<VoxelGrid<glm::ivec3>> readProbeDataToFile(const std::string& path,
		std::vector<ReflectionProbe>& outProbes, int& outTextureSize, int& outNumBounces) {
	std::ifstream inputFile(path, std::ios::binary | std::ios::in);

	std::uint32_t numProbes;
	inputFile.read(reinterpret_cast<char*>(&numProbes), sizeof(std::uint32_t));
	inputFile.read(reinterpret_cast<char*>(&outTextureSize), sizeof(std::int32_t));
	inputFile.read(reinterpret_cast<char*>(&outNumBounces), sizeof(std::int32_t));

	outProbes.reserve(numProbes);
	for (std::uint32_t i = 0; i < numProbes; ++i) {
		ReflectionProbe probe;

		inputFile.read(reinterpret_cast<char*>(&probe.position.x), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.position.y), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.position.z), sizeof(float));

		inputFile.read(reinterpret_cast<char*>(&probe.aabbMin.x), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.aabbMin.y), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.aabbMin.z), sizeof(float));

		inputFile.read(reinterpret_cast<char*>(&probe.aabbMax.x), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.aabbMax.y), sizeof(float));
		inputFile.read(reinterpret_cast<char*>(&probe.aabbMax.z), sizeof(float));

		inputFile.read(reinterpret_cast<char*>(&probe.layer), sizeof(std::uint32_t));

		outProbes.push_back(probe);
	}

	glm::ivec3 dim;
	inputFile.read(reinterpret_cast<char*>(&dim.x), sizeof(std::int32_t));
	inputFile.read(reinterpret_cast<char*>(&dim.y), sizeof(std::int32_t));
	inputFile.read(reinterpret_cast<char*>(&dim.z), sizeof(std::int32_t));

	glm::vec3 min;
	inputFile.read(reinterpret_cast<char*>(&min.x), sizeof(float));
	inputFile.read(reinterpret_cast<char*>(&min.y), sizeof(float));
	inputFile.read(reinterpret_cast<char*>(&min.z), sizeof(float));

	glm::vec3 max;
	inputFile.read(reinterpret_cast<char*>(&max.x), sizeof(float));
	inputFile.read(reinterpret_cast<char*>(&max.y), sizeof(float));
	inputFile.read(reinterpret_cast<char*>(&max.z), sizeof(float));

	std::vector<glm::ivec3> gridData(dim.x * dim.y * dim.z);
	inputFile.read(reinterpret_cast<char*>(gridData.data()), sizeof(glm::ivec3) * dim.x * dim.y * dim.z);

	inputFile.close();

	std::shared_ptr<VoxelGrid<glm::ivec3>> visibilityGrid = std::make_shared<VoxelGrid<glm::ivec3>>(min, max, dim);
	visibilityGrid->forEachVoxel([&](int x, int y, int z) {
		int index = visibilityGrid->getVoxelIndex({ x, y, z });
		visibilityGrid->setVoxel({ x, y, z }, gridData[index]);
	});

	return visibilityGrid;
}