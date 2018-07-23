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
        return (barycentric.x >= 0.0f) && (barycentric.y >= 0.0f) && (barycentric.x + barycentric.y <= 1.0f);
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

SharedImage IlluminationBaker::bakeIrradiance(const Primitive& primitive, int width, int height, int samplesPerTexel) {
	auto values = bake(primitive, width, height, samplesPerTexel, [&](glm::vec3 pos, glm::vec3 normal) {
		glm::vec3 dir = sampleCosineHemisphere(normal);
		glm::vec3 irradiance = pathTracer->trace(pos, dir); // dot(N,L) and pdf canceled
		return irradiance;
	});

	SharedImage bakedMap = std::make_shared<Image>(width, height, GL_RGB16F);
	for (int i = 0; i < width * height; ++i) {
		bakedMap->getDataPtr<glm::u16vec3>()[i] = glm::packHalf(values[i]);
	}
	return bakedMap;
}

SharedImage IlluminationBaker::bakeAmbientOcclusion(const Primitive& primitive, int width, int height,
													int samplesPerTexel, float maxDistance) {
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
											   int samplesPerTexel, const BakeOperator& op) {
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

		glm::vec2 texel0 = t0 * glm::vec2(width - 1, height - 1) + glm::vec2(0.5f);
		glm::vec2 texel1 = t1 * glm::vec2(width - 1, height - 1) + glm::vec2(0.5f);
		glm::vec2 texel2 = t2 * glm::vec2(width - 1, height - 1) + glm::vec2(0.5f);

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

		int numStepsX = static_cast<int>(std::ceil(maxX - minX));
		int numStepsY = static_cast<int>(std::ceil(maxY - minY));
		//if (static_cast<int>(minX) + numStepsX >= width) numStepsX = width - static_cast<int>(minX);// -1;
		//if (static_cast<int>(minY) + numStepsY >= height) numStepsY = height - static_cast<int>(minY);// -1;

		for (int sample = 0; sample < samplesPerTexel; ++sample) {
			#pragma omp parallel for
			for (int stepY = 0; stepY <= numStepsY; ++stepY) {
				for (int stepX = 0; stepX <= numStepsX; ++stepX) {
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
					
					int imageX = static_cast<int>(texelP.x - 0.5f);
					int imageY = static_cast<int>(texelP.y - 0.5f);
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