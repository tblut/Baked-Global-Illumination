#pragma once

#include "Primitive.hh"
#include "DirectionalLight.hh"

#include <embree3/rtcore.h>
#include <glow/fwd.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

class PathTracer {
public:
	PathTracer();
	virtual ~PathTracer();
	
	void buildScene(const std::vector<Primitive>& primitives);
	glm::vec3 trace(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& weight = glm::vec3(1.0f), int depth = 0) const;
	float testOcclusionDist(const glm::vec3& origin, const glm::vec3& dir) const;
	void setLight(const DirectionalLight& light);
	void setMaxPathDepth(unsigned int depth);
	void setClampDepth(unsigned int depth);
	void setClampRadiance(float radiance);

private:
	struct Triangle {
		unsigned int v0;
		unsigned int v1;
		unsigned int v2;
	};

	struct Material {
		SharedImage albedoMap;
		SharedImage normalMap;
		SharedImage roughnessMap;
		glm::vec3 baseColor;
		float roughness;
		float metallic;
	};

	RTCDevice device = nullptr;
	RTCScene scene = nullptr;
	std::unordered_map<unsigned int, Material> materials;
	const DirectionalLight* light = nullptr;
	int maxPathDepth = 5;
	int clampDepth = 0;
	float clampRadiance = 25.0f;
};
