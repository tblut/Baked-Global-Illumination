#pragma once

#include "Scene.hh"
#include "RenderPipeline.hh"
#include "DebugPathTracer.hh"
#include "IlluminationBaker.hh"
#include "ReflProbeBaker.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/glfw/GlfwApp.hh>

#include <memory>

class BakedGIApp : public glow::glfw::GlfwApp {
public:
	BakedGIApp(const std::string& gltfPath, const std::string& lmPath);

protected:
	virtual void init() override;
	virtual void render(float elapsedSeconds) override;
	virtual void onResize(int w, int h) override;

private:
	std::string gltfPath;
	std::string lmPath;
	Scene scene;
	std::unique_ptr<RenderPipeline> pipeline;
	std::unique_ptr<DebugPathTracer> debugPathTracer;
    std::unique_ptr<IlluminationBaker> illuminationBaker;
     std::unique_ptr<ReflProbeBaker> reflProbeBaker;
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
	bool showProbeGrid = false;
	bool showProbeVisGrid = false;
};
