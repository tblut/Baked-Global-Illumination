#include "LightMapBaker.hh"
#include "Primitive.hh"
#include "PathTracer.hh"

#include <glow/common/log.hh>
#include <random>

namespace {
    glm::vec2 getBarycentricCoords(const glm::vec2& p, const glm::vec2& a,
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
        
        return glm::vec2(u, v);
    }
    
    bool isPointInTriangle(const glm::vec2& barycentric) {
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
}

LightMapBaker::LightMapBaker(const PathTracer& pathTracer) : pathTracer(&pathTracer) {

}

SharedImage LightMapBaker::bake(const Primitive& primitive, int width, int height) {
	SharedImage lightMap = std::make_shared<Image>(width, height, 3, GL_FLOAT, glow::ColorSpace::Linear);
    
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(primitive.transform)));
    glm::vec2 pixelSize = glm::vec2(1.0f) / glm::vec2(static_cast<float>(width), static_cast<float>(height)); 

    for (std::size_t i = 0; i < primitive.indices.size(); i += 3) {
        glm::vec2 t0 = primitive.lightMapTexCoords[primitive.indices[i]];
        glm::vec2 t1 = primitive.lightMapTexCoords[primitive.indices[i + 1]];
        glm::vec2 t2 = primitive.lightMapTexCoords[primitive.indices[i + 2]];
        
        glm::vec3 v0 = primitive.positions[primitive.indices[i]];
        glm::vec3 v1 = primitive.positions[primitive.indices[i + 1]];
        glm::vec3 v2 = primitive.positions[primitive.indices[i + 2]];
        
        glm::vec3 n0 = primitive.normals[primitive.indices[i]];
        glm::vec3 n1 = primitive.normals[primitive.indices[i + 1]];
        glm::vec3 n2 = primitive.normals[primitive.indices[i + 2]];
        
        float minX = std::min(t0.x, std::min(t1.x, t2.x));
        float minY = std::min(t0.y, std::min(t1.y, t2.y));
        float maxX = std::max(t0.x, std::max(t1.x, t2.x));
        float maxY = std::max(t0.y, std::max(t1.y, t2.y));
        
        int numStepsX = (maxX - minX) / pixelSize.x;
        int numStepsY = (maxY - minY) / pixelSize.y;
        
        #pragma omp parallel for
        for (int stepY = 0; stepY < numStepsX; ++stepY) {
            for (int stepX = 0; stepX < numStepsX; ++stepX) {
                float x = minX + stepX * pixelSize.x;
                float y = minY + stepY * pixelSize.y;
                
                glm::vec2 barycentric = getBarycentricCoords(glm::vec2(x, y), t0, t1, t2);
                if (!isPointInTriangle(barycentric)) {
                    continue;
                }

                glm::vec3 localPos = v0 * barycentric.x
                    + v1 * barycentric.y
                    + v2 * (1.0f - barycentric.x - barycentric.y);
                glm::vec3 worldPos = primitive.transform * glm::vec4(localPos, 1.0f);
  
                glm::vec3 localNormal = n0 * barycentric.x
                    + n1 * barycentric.y
                    + n2 * (1.0f - barycentric.x - barycentric.y);
                glm::vec3 worldNormal = glm::normalize(normalMatrix * glm::normalize(localNormal));

                glm::vec3 irradiance(0.0f);
                const int numSamples = 100;
                for (int sample = 0; sample < numSamples; ++sample) {
                    glm::vec3 dir = sampleCosineHemisphere(worldNormal);
                    irradiance += pathTracer->trace(worldPos, dir);
                }
                irradiance /= numSamples;
                
                irradiance = irradiance / (irradiance + glm::vec3(1.0f)); // Tone mapping
                
                int imageX = static_cast<int>(x * width);
                int imageY = static_cast<int>(y * height);
                
                lightMap->getDataPtr<glm::vec3>()[imageX + imageY * width] = irradiance;
            }
        }
    }
    
    return lightMap;
}
