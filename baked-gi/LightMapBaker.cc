#include "LightMapBaker.hh"
#include "Primitive.hh"
#include "PathTracer.hh"

#include <glow/common/log.hh>
#include <random>

namespace {
    glm::vec3 getBarycentricCoords(const glm::vec2& p, const glm::vec2& a,
                                   const glm::vec2& b, const glm::vec2& c) {
        glm::vec2 v0 = c - a;
        glm::vec2 v1 = b - a;
        glm::vec2 v2 = p - a;

        float dot00 = glm::dot(v0, v0);
        float dot01 = glm::dot(v0, v1);
        float dot02 = glm::dot(v0, v2);
        float dot11 = glm::dot(v1, v1);
        float dot12 = glm::dot(v1, v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        
        return glm::vec3(u, v, 1.0f - u - v);
    }
    
    bool isPointInTriangle(const glm::vec3& barycentric) {
        return (barycentric.x >= 0.0f) && (barycentric.y >= 0.0f) && (barycentric.x + barycentric.y < 1.0f);
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
		float z = std::sqrt(1.0f - x * x - y * y);

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
    
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(primitive.transform)));
    glm::vec2 texelSize = glm::vec2(1.0f) / glm::vec2(static_cast<float>(width), static_cast<float>(height)); 

    for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
        glm::vec2 t0 = primitive.lightMapTexCoords[primitive.indices[i]];
        glm::vec2 t1 = primitive.lightMapTexCoords[primitive.indices[i + 1]];
        glm::vec2 t2 = primitive.lightMapTexCoords[primitive.indices[i + 2]];
        
        glm::vec3 v0 = primitive.transform * glm::vec4(primitive.positions[primitive.indices[i]], 1.0f);
        glm::vec3 v1 = primitive.transform * glm::vec4(primitive.positions[primitive.indices[i + 1]], 1.0f);
        glm::vec3 v2 = primitive.transform * glm::vec4(primitive.positions[primitive.indices[i + 2]], 1.0f);
        
        glm::vec3 n0 = normalMatrix * primitive.normals[primitive.indices[i]];
        glm::vec3 n1 = normalMatrix * primitive.normals[primitive.indices[i + 1]];
        glm::vec3 n2 = normalMatrix * primitive.normals[primitive.indices[i + 2]];
        
        glm::vec2 texel0 = t0 + glm::vec2(0.5f) * texelSize;
        glm::vec2 texel1 = t1 + glm::vec2(0.5f) * texelSize;
        glm::vec2 texel2 = t2 + glm::vec2(0.5f) * texelSize;
        
        float minX = std::min(texel0.x, std::min(texel1.x, texel2.x));
        float minY = std::min(texel0.y, std::min(texel1.y, texel2.y));
        float maxX = std::max(texel0.x, std::max(texel1.x, texel2.x));
        float maxY = std::max(texel0.y, std::max(texel1.y, texel2.y));
        
        int numStepsX = static_cast<int>(std::ceil((maxX - minX) / texelSize.x));
        int numStepsY = static_cast<int>(std::ceil((maxY - minY) / texelSize.y));
        
        std::vector<glm::vec3> colors(width * height);
        
        const int numSamples = 100;
        for (int sample = 0; sample < numSamples; ++sample) {
        
            //#pragma omp parallel for
            for (int stepY = 0; stepY < numStepsY; ++stepY) {
                for (int stepX = 0; stepX < numStepsX; ++stepX) {
                    glm::vec2 texelP = glm::vec2(minX, minY) + glm::vec2(stepX, stepY) * texelSize;

                    float xx = texelP.x;// + (uniformDist(randEngine) - 0.5f) * texelSize.x;
                    float yy = texelP.y;// + (uniformDist(randEngine) - 0.5f) * texelSize.y;
                    
                    glm::vec3 bary = getBarycentricCoords(glm::vec2(xx, yy), texel0, texel1, texel2);
                    if (!isPointInTriangle(bary)) {
                        continue;
                    }
                    
                    glm::vec3 worldPos = v0 * bary.x + v1 * bary.y + v2 * bary.z;
                    glm::vec3 worldNormal = glm::normalize(n0 * bary.x + n1 * bary.y + n2 * bary.z);

                    //glow::info() << worldNormal.x << " " << worldNormal.y << " " << worldNormal.z;
                    
                    glm::vec3 dir = sampleCosineHemisphere(worldNormal);
                    glm::vec3 irradiance = pathTracer->trace(worldPos, dir) / pdfCosineHemisphere(worldNormal, dir);

                    //irradiance = irradiance / (irradiance + glm::vec3(1.0f)); // Tone mapping
                    
                    int imageX = static_cast<int>(texelP.x * width - 0.5f);
                    int imageY = static_cast<int>(texelP.y * height - 0.5f);
                    colors[imageX + imageY * width] += irradiance;
                }
            }
        }
        
        for (int i = 0; i < width * height; ++i) {
            lightMap->getDataPtr<glm::vec3>()[i] = colors[i] / static_cast<float>(numSamples);
        }
    }
    
    return lightMap;
}
