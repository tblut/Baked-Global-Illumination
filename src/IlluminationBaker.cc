#include "IlluminationBaker.hh"
#include "Primitive.hh"
#include "PathTracer.hh"

#include <glow/common/log.hh>
#include <glm/gtc/packing.hpp>
#include <random>

namespace {
    glm::vec3 getBarycentricCoords(const glm::vec2& p, const glm::vec2& a,
                                   const glm::vec2& b, const glm::vec2& c) {
        glm::vec2 v0 = b - a;
        glm::vec2 v1 = c - a;
        glm::vec2 v2 = p - a;

        float dot00 = glm::dot(v0, v0);
        float dot01 = glm::dot(v0, v1);
		float dot11 = glm::dot(v1, v1);
        float dot20 = glm::dot(v2, v0);
        float dot21 = glm::dot(v2, v1);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float v = (dot11 * dot20 - dot01 * dot21) * invDenom;
        float w = (dot00 * dot21 - dot01 * dot20) * invDenom;
		float u = 1.0f - v - w;
        
        return glm::vec3(u, v, w);
    }
    
    bool isPointInTriangle(const glm::vec3& barycentric) {
        return (barycentric.y >= 0.0f) && (barycentric.z >= 0.0f) && (barycentric.y + barycentric.z <= 1.0f);
    }
    
    std::random_device randDevice;
	std::default_random_engine randEngine(randDevice());
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);

	void makeCoordinateSystem(const glm::vec3& normal, glm::vec3& xAxis, glm::vec3& yAxis) {
		xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		if (std::abs(1.0f - normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, -1.0f);
		}
		else if (std::abs(1.0f + normal.x) < 1.0e-8f) {
			xAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		yAxis = glm::normalize(glm::cross(normal, xAxis));
		xAxis = glm::normalize(glm::cross(yAxis, normal));
	}

	glm::vec3 sampleCosineHemisphere(const glm::vec3& normal) {
		float u1 = uniformDist(randEngine);
		float u2 = uniformDist(randEngine);
		
		float r = std::sqrt(u1);
		float phi = 2.0f * glm::pi<float>() * u2;

		float x = r * std::sin(phi);
		float y = r * std::cos(phi);
		float z = std::sqrt(std::max(0.0f, 1.0f - u1));

		glm::vec3 xAxis;
		glm::vec3 yAxis;
		makeCoordinateSystem(normal, xAxis, yAxis);

		return glm::normalize(x * xAxis + y * yAxis + z * normal);
	}

	float pdfCosineHemisphere(const glm::vec3& normal, const glm::vec3& wi) {
		return glm::dot(normal, wi) / glm::pi<float>();
	}
}

IlluminationBaker::IlluminationBaker(const PathTracer& pathTracer) : pathTracer(&pathTracer) {

}

SharedImage IlluminationBaker::bakeIrradiance(const Primitive& primitive, int width, int height, int samplesPerTexel) const {
	auto values = bake(primitive, width, height, samplesPerTexel, [&](glm::vec3 pos, glm::vec3 normal) {
		glm::vec3 dir = sampleCosineHemisphere(normal);
		glm::vec3 irradiance = pathTracer->trace(pos, dir); // dot(N,L) and pdf canceled
		return irradiance;
	});

	fillIllegalTexels(primitive, width, height, values);

	SharedImage bakedMap = std::make_shared<Image>(width, height, GL_RGB16F);
	for (int i = 0; i < width * height; ++i) {
		bakedMap->getDataPtr<glm::u16vec3>()[i] = glm::packHalf(values[i]);
	}
	return bakedMap;
}

SharedImage IlluminationBaker::bakeAmbientOcclusion(const Primitive& primitive, int width, int height,
													int samplesPerTexel, float maxDistance) const {
	auto values = bake(primitive, width, height, samplesPerTexel, [&](glm::vec3 pos, glm::vec3 normal) {
		glm::vec3 dir = sampleCosineHemisphere(normal);
		float occlusionDist = pathTracer->testOcclusionDist(pos, dir);
		float occlusion(1.0f);
		if (occlusionDist > 0.0f) {
			float atten = std::max(0.0f, maxDistance - occlusionDist) / maxDistance;
			occlusion = 0.0f + atten * atten;
		}
		return glm::vec3(occlusion);
	});

	SharedImage bakedMap = std::make_shared<Image>(width, height, GL_R16F);
	for (int i = 0; i < width * height; ++i) {
		bakedMap->getDataPtr<glm::uint16>()[i] = glm::packHalf1x16(values[i].x);
	}
	return bakedMap;
}

std::vector<glm::vec3> IlluminationBaker::bake(const Primitive& primitive, int width, int height,
											   int samplesPerTexel, const BakeOperator& op) const {
	std::vector<glm::vec3> buffer(width * height, glm::vec3(0.0f));
	std::vector<int> numSamples(width * height, 0);
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(primitive.transform)));

	for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
		unsigned int index0 = primitive.indices[i];
		unsigned int index1 = primitive.indices[i + 1];
		unsigned int index2 = primitive.indices[i + 2];

		glm::vec2 t0 = primitive.lightMapTexCoords[index0];
		glm::vec2 t1 = primitive.lightMapTexCoords[index1];
		glm::vec2 t2 = primitive.lightMapTexCoords[index2];

		if (t0.x < 0 || t0.x > 1 || t0.y < 0 || t0.y > 1
				|| t1.x < 0 || t1.x > 1 || t1.y < 0 || t1.y > 1
				|| t2.x < 0 || t2.x > 1 || t2.y < 0 || t2.y > 1) {
			glow::error() << "The light map UV coordinates are not in the [0,1] range for " << primitive.name;
		}

		glm::vec2 texel0 = glm::floor(t0 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);
		glm::vec2 texel1 = glm::floor(t1 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);
		glm::vec2 texel2 = glm::floor(t2 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);

		glm::vec3 v0 = primitive.transform * glm::vec4(primitive.positions[index0], 1.0f);
		glm::vec3 v1 = primitive.transform * glm::vec4(primitive.positions[index1], 1.0f);
		glm::vec3 v2 = primitive.transform * glm::vec4(primitive.positions[index2], 1.0f);

		glm::vec3 n0 = glm::normalize(normalMatrix * primitive.normals[index0]);
		glm::vec3 n1 = glm::normalize(normalMatrix * primitive.normals[index1]);
		glm::vec3 n2 = glm::normalize(normalMatrix * primitive.normals[index2]);

		float minX = std::min(texel0.x, std::min(texel1.x, texel2.x));
		float minY = std::min(texel0.y, std::min(texel1.y, texel2.y));
		float maxX = std::max(texel0.x, std::max(texel1.x, texel2.x));
		float maxY = std::max(texel0.y, std::max(texel1.y, texel2.y));

		int numStepsX = static_cast<int>(maxX) - static_cast<int>(minX) + 1;
		int numStepsY = static_cast<int>(maxY) - static_cast<int>(minY) + 1;

		for (int sample = 0; sample < samplesPerTexel; ++sample) {
			#pragma omp parallel for
			for (int stepY = 0; stepY < numStepsY; ++stepY) {
				for (int stepX = 0; stepX < numStepsX; ++stepX) {
					glm::vec2 texelP = glm::vec2(minX, minY) + glm::vec2(stepX, stepY);
					texelP.x = texelP.x + (uniformDist(randEngine) - 0.5f);
					texelP.y = texelP.y + (uniformDist(randEngine) - 0.5f);

					glm::vec3 bary = getBarycentricCoords(texelP, texel0, texel1, texel2);
					if (!isPointInTriangle(bary)) {
						continue;
					}

					glm::vec3 worldPos = v0 * bary.x + v1 * bary.y + v2 * bary.z;
					glm::vec3 worldNormal = glm::normalize(n0 * bary.x + n1 * bary.y + n2 * bary.z);
					auto value = op(worldPos, worldNormal);
					
					int imageX = static_cast<int>(texelP.x);
					int imageY = static_cast<int>(texelP.y);
					buffer[imageX + imageY * width] += value;
					numSamples[imageX + imageY * width]++;
				}
			}
		}
	}

	for (int k = 0; k < buffer.size(); ++k) {
		buffer[k] /= static_cast<float>(std::max(1, numSamples[k]));
	}

	return buffer;
}

void IlluminationBaker::fillIllegalTexels(const Primitive& primitive, int width, int height, std::vector<glm::vec3>& values) const {
	std::vector<bool> illegalMap(width * height, true);

	for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
		unsigned int index0 = primitive.indices[i];
		unsigned int index1 = primitive.indices[i + 1];
		unsigned int index2 = primitive.indices[i + 2];

		glm::vec2 t0 = primitive.lightMapTexCoords[index0];
		glm::vec2 t1 = primitive.lightMapTexCoords[index1];
		glm::vec2 t2 = primitive.lightMapTexCoords[index2];

		glm::vec2 texel0 = glm::floor(t0 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);
		glm::vec2 texel1 = glm::floor(t1 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);
		glm::vec2 texel2 = glm::floor(t2 * glm::vec2(width - 1, height - 1)) + glm::vec2(0.5f);

		float minX = std::min(texel0.x, std::min(texel1.x, texel2.x));
		float minY = std::min(texel0.y, std::min(texel1.y, texel2.y));
		float maxX = std::max(texel0.x, std::max(texel1.x, texel2.x));
		float maxY = std::max(texel0.y, std::max(texel1.y, texel2.y));

		int numStepsX = static_cast<int>(maxX) - static_cast<int>(minX) + 1;
		int numStepsY = static_cast<int>(maxY) - static_cast<int>(minY) + 1;

		int numSamplesX = 8;
		int numSamplesY = 8;
		glm::vec2 offset = glm::vec2(1.0f / numSamplesX, 1.0f / numSamplesY);
		glm::vec2 stepSize = (glm::vec2(1.0f) - offset * 2.0f) / glm::vec2(numSamplesX, numSamplesY);

		for (int stepY = 0; stepY < numStepsY; ++stepY) {
			for (int stepX = 0; stepX < numStepsX; ++stepX) {
				glm::vec2 texelCenter = glm::vec2(minX, minY) + glm::vec2(stepX, stepY);
				int imageX = static_cast<int>(texelCenter.x);
				int imageY = static_cast<int>(texelCenter.y);

				for (int y = 0; y < numSamplesY; ++y) {
					for (int x = 0; x < numSamplesX; ++x) {
						glm::vec2 samplePoint = texelCenter - glm::vec2(0.5f) + offset + stepSize * glm::vec2(x, y);
						if (isPointInTriangle(getBarycentricCoords(samplePoint, texel0, texel1, texel2))) {
							illegalMap[imageX + imageY * width] = false;
							break;
						}
					}

					if (!illegalMap[imageX + imageY * width]) {
						break;
					}
				}
			}
		}
	}

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			if (!illegalMap[x + y * width]) {
				continue;
			}

			glm::ivec2 legalCoord = findClosestLegalTexel(x, y, width, height, illegalMap);
			if (legalCoord.x != -1 && legalCoord.y != -1) {
				values[x + y * width] = values[legalCoord.x + legalCoord.y * width];
			}
		}
	}
}

glm::ivec2 IlluminationBaker::findClosestLegalTexel(int x, int y, int width, int height, const std::vector<bool>& illegalMap) const {
	glm::ivec2 offsets[] = {
		{ -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, // Distance = 1 * offsetScale
		{ -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } // Distance = sqrt(2) * offsetScale
	};
	int offsetScale = 1;

	bool isOutOfBounds = false;
	while (!isOutOfBounds) {
		for (auto offset : offsets) {
			glm::ivec2 coord = glm::ivec2(x, y) + offset * offsetScale;
			if ((coord.x < 0 || coord.x >= width) && (coord.y < 0 || coord.y >= height)) {
				isOutOfBounds = true;
				continue;
			}

			coord.x = std::max(0, std::min(width - 1, coord.x));
			coord.y = std::max(0, std::min(height - 1, coord.y));
			isOutOfBounds = false;

			if (!illegalMap[coord.x + coord.y * width]) {
				return coord;
			}
		}

		offsetScale++;
	}

	return glm::ivec2(-1);
}
