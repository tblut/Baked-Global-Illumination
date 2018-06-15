#include "LightMapBaker.hh"
#include "Primitive.hh"
#include "PathTracer.hh"

#include <glow/common/log.hh>
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

LightMapBaker::LightMapBaker(const PathTracer& pathTracer) : pathTracer(&pathTracer) {

}

SharedImage LightMapBaker::bake(const Primitive& primitive, int width, int height) {
	SharedImage lightMap = std::make_shared<Image>(width, height, GL_RGB32F);

	const int numSamples = 5000;
	std::vector<glm::vec3> colors(width * height, glm::vec3(0.0f));
	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(primitive.transform)));

    for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
		unsigned int index0 = primitive.indices[i];
		unsigned int index1 = primitive.indices[i + 1];
		unsigned int index2 = primitive.indices[i + 2];

        glm::vec2 t0 = primitive.lightMapTexCoords[index0];
        glm::vec2 t1 = primitive.lightMapTexCoords[index1];
        glm::vec2 t2 = primitive.lightMapTexCoords[index2];

		glm::vec2 texel0 = t0 * glm::vec2(width, height) + glm::vec2(0.5f);
		glm::vec2 texel1 = t1 * glm::vec2(width, height) + glm::vec2(0.5f);
		glm::vec2 texel2 = t2 * glm::vec2(width, height) + glm::vec2(0.5f);
		
        glm::vec3 v0 = primitive.transform * glm::vec4(primitive.positions[index0], 1.0f);
        glm::vec3 v1 = primitive.transform * glm::vec4(primitive.positions[index1], 1.0f);
        glm::vec3 v2 = primitive.transform * glm::vec4(primitive.positions[index2], 1.0f);
        
        glm::vec3 n0 = normalMatrix * primitive.normals[index0];
        glm::vec3 n1 = normalMatrix * primitive.normals[index1];
        glm::vec3 n2 = normalMatrix * primitive.normals[index2];
        
        float minX = std::min(texel0.x, std::min(texel1.x, texel2.x));
		float minY = std::min(texel0.y, std::min(texel1.y, texel2.y));
		float maxX = std::max(texel0.x, std::max(texel1.x, texel2.x));
		float maxY = std::max(texel0.y, std::max(texel1.y, texel2.y));
        
        int numStepsX = static_cast<int>(std::ceil(maxX - minX));
        int numStepsY = static_cast<int>(std::ceil(maxY - minY));
		if (minX + numStepsX >= width) numStepsX = width - minX - 1;
		if (minY + numStepsY >= height) numStepsY = height - minY - 1;

		for (int sample = 0; sample < numSamples; ++sample) {
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

					glm::vec3 dir = sampleCosineHemisphere(worldNormal);
					glm::vec3 radiance = pathTracer->trace(worldPos, dir);

					int imageX = static_cast<int>(texelP.x - 0.5f);
					int imageY = static_cast<int>(texelP.y - 0.5f);
					colors[imageX + imageY * width] += radiance;
				}
			}
		}
    }
    
	for (int k = 0; k < colors.size(); ++k) {
		lightMap->getDataPtr<glm::vec3>()[k] = colors[k] / static_cast<float>(numSamples);
	}

    return lightMap;
}
