#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

template <typename T>
class VoxelGrid {
public:
	VoxelGrid(glm::vec3 min, glm::vec3 max, glm::ivec3 dimensions) {
		voxelSize = (max - min) / glm::vec3(dimensions);
		gridDimensions = dimensions;
		gridMin = min;
		gridMax = max;
		grid.resize(dimensions.x * dimensions.y * dimensions.z);
	}

	void fill(const T& value) {
		for (std::size_t i = 0; i < grid.size(); ++i) {
			grid[i] = value;
		}
	}

	void setVoxel(glm::ivec3 coord, const T& value) {
		grid[getVoxelIndex(coord)] = value;
	}

	const T& getVoxel(glm::ivec3 coord) const {
		return grid[getVoxelIndex(coord)];
	}

	glm::ivec3 getVoxelCoord(glm::vec3 worldPos) const {
		return glm::ivec3(worldPos / voxelSize);
	}

	glm::vec3 getVoxelMin(glm::ivec3 coord) const {
		return gridMin + glm::vec3(coord) * voxelSize;
	}

	glm::vec3 getVoxelMax(glm::ivec3 coord) const {
		return getVoxelMin(coord) + voxelSize;
	}

	glm::vec3 getVoxelCenter(glm::ivec3 coord) const {
		return getVoxelMin(coord) + voxelSize * 0.5f;
	}

	glm::vec3 getVoxelSize() const {
		return voxelSize;
	}

	glm::ivec3 getDimensions() const {
		return gridDimensions;
	}

	glm::vec3 getMin() const {
		return gridMin;
	}

	glm::vec3 getMax() const {
		return gridMax;
	}

	const std::vector<T> getInternalArray() const {
		return grid;
	}
	
	int getVoxelIndex(glm::ivec3 coord) const {
		return coord.x + coord.y * gridDimensions.x + coord.z * gridDimensions.x * gridDimensions.y;
	}

	void forEachVoxel(const std::function<void(int, int, int)>& body) {
		for (int z = 0; z < gridDimensions.z; ++z) {
			for (int y = 0; y < gridDimensions.y; ++y) {
				for (int x = 0; x < gridDimensions.x; ++x) {
					body(x, y, z);
				}
			}
		}
	}

private:
	glm::vec3 gridMin;
	glm::vec3 gridMax;
	glm::vec3 voxelSize;
	glm::ivec3 gridDimensions;
	std::vector<T> grid;
};