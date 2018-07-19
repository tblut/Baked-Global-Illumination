#pragma once

#include "Scene.hh"
#include "RenderPipeline.hh"
#include "DebugPathTracer.hh"
#include "IlluminationBaker.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/glfw/GlfwApp.hh>
#include <AntTweakBar.h>

#include <memory>

class BakedGIApp : public glow::glfw::GlfwApp {
public:
	BakedGIApp(const std::string& gltfPath, const std::string& lmPath, const std::string& pdPath);

protected:
	virtual void init() override;
	virtual void render(float elapsedSeconds) override;
	virtual void onResize(int w, int h) override;

private:
	struct SharedData {
		const Scene* scene;
		RenderPipeline* pipeline;
		DebugPathTracer* pathTracer;
		glow::camera::SharedGenericCamera camera;
		std::vector<ReflectionProbe>* probes;
		int numBounces = 2;
		int probeSize = 128;
		bool isInProbePlacementMode = false;
		int currentProbeIndex = -1;
		glm::vec3 currentProbePos = glm::vec3(0);
		glm::vec3 currentProbeAABBMin = glm::vec3(-2);
		glm::vec3 currentProbeAABBMax = glm::vec3(2);
		int voxelGridRes = 128;
		std::shared_ptr<VoxelGrid<glm::ivec3>> visibilityGrid;
	} sharedData;

	static void TW_CALL debugTrace(void* clientData);
	static void TW_CALL saveTrace(void* clientData);
	static void TW_CALL placeProbe(void* clientData);
	static void TW_CALL removeProbe(void* clientData);
	static void TW_CALL rebakeProbes(void* clientData);
	static void TW_CALL saveProbeData(void* clientData);

	std::string gltfPath;
	std::string lmPath;
	std::string pdPath;
	Scene scene;
	std::unique_ptr<RenderPipeline> pipeline;
	std::unique_ptr<DebugPathTracer> debugPathTracer;
    std::unique_ptr<IlluminationBaker> illuminationBaker;
	bool showDebugImage = false;
    float lastDebugTraceScale = -1.0f; // force update on first frame
	float debugTraceScale = 0.5f;
	unsigned int samplesPerPixel = 100;
	unsigned int maxPathDepth = 3;
	unsigned int clampDepth = 0;
	float clampRadiance = 25.0f;
	bool showDebugLightMap = false;
	int lightMapIndex = 0;
	int shadowMapSize = 4096;
	float shadowMapOffset = 0.001f;
	bool useIrradianceMap = true;
	bool useAOMap = true;
	float bloomPercentage = 0.02f;
	float exposureAdjustment = 1.0f;
	int debugEnvMapMipLevel = 0;
	bool showDebugEnvProbes = false;
	bool useIbl = true;
	bool useLocalProbes = true;
	bool showProbeVisGrid = false;

	std::vector<ReflectionProbe> reflectionProbes;
	int lastProbeIndex = -100;
};
