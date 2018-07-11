#include "BakedGIApp.hh"
#include "LightMapWriter.hh"

#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/TextureCubeMap.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/data/TextureData.hh>
#include <glow/common/str_utils.hh>
#include <glow/common/scoped_gl.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/assimp/Importer.hh>
#include <glow-extras/debugging/DebugRenderer.hh>
#include <AntTweakBar.h>
#include <embree3/rtcore.h>
#include <fstream>

namespace {
	struct DebugProbeData {
		RenderPipeline* pipeline;
		Scene* scene;
		glow::camera::SharedGenericCamera camera;
	};

	void debugTrace(void* clientData) {
		DebugPathTracer* tracer = static_cast<DebugPathTracer*>(clientData);
		tracer->traceDebugImage();
	}

	void saveTrace(void* clientData) {
		DebugPathTracer* tracer = static_cast<DebugPathTracer*>(clientData);
		tracer->saveDebugImageToFile(glow::util::pathOf(__FILE__) + "/textures/debugtrace.png");
	}

	DebugProbeData probeData;
	void makeDebugProbe(void* clientData) {
		glm::vec3 envMapPos = probeData.camera->getPosition();
		auto envMap = probeData.pipeline->renderEnvironmentMap(envMapPos, 256, probeData.scene->getMeshes());
		//envMap->bind().generateMipmaps();
		//probeData.pipeline->setDebugEnvMap(envMap, envMapPos);
        probeData.pipeline->setProbes(envMapPos, glm::vec3(10));
	}
}

BakedGIApp::BakedGIApp(const std::string& gltfPath, const std::string& lmPath) {
	this->gltfPath = gltfPath;
	this->lmPath = lmPath;
}

void BakedGIApp::init() {
	glow::glfw::GlfwApp::init();
    
    GLint numLayers;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &numLayers);
    glow::info() << "Max array texture layers: " << numLayers;

	this->setQueryStats(false);

	auto cam = getCamera();
	cam->setPosition({ 0, 0, 1 });
	cam->setTarget({ 0, 0, 0 }, { 0, 1, 0 });

	scene.loadFromGltf(gltfPath);// glow::util::pathOf(__FILE__) + "/models/test2.glb");

	pipeline.reset(new RenderPipeline());
	pipeline->attachCamera(*getCamera());
	pipeline->attachLight(scene.getSun());

	debugPathTracer.reset(new DebugPathTracer());
	debugPathTracer->attachDebugCamera(*getCamera());
	scene.buildPathTracerScene(*debugPathTracer);
    
    auto pbt = glow::util::pathOf(__FILE__) + "/textures/miramar";
	auto skybox = CubeMap::loadFromFiles(
		pbt + "/posx.jpg",
		pbt + "/negx.jpg",
		pbt + "/posy.jpg",
		pbt + "/negy.jpg",
		pbt + "/posz.jpg",
		pbt + "/negz.jpg");
	debugPathTracer->setBackgroundCubeMap(skybox);
    
    reflProbeBaker.reset(new ReflProbeBaker(*pipeline, *debugPathTracer));
    reflProbeBaker->generateEmptyProbeGrid(scene, { 3, 2, 3 });
	scene.buildRealtimeObjects(lmPath, reflProbeBaker->computePrimitiveProbeIndices(scene));
    
    //pipeline->makeDebugReflProbeGrid(scene, 2, 2, 2);
    //pipeline->setDebugReflProbeGridEnabled(true);
    
    reflProbeBaker->bakeGGXEnvProbes(scene, 128);
    pipeline->setReflectionProbes(reflProbeBaker->getReflectionProbes());

	
	//TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &ambientColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &scene.getSun().color, "group=light");
	TwAddVarRW(tweakbar(), "Light Power", TW_TYPE_FLOAT, &scene.getSun().power, "group=light min=0.1 step=0.1");
	TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &scene.getSun().direction, "group=light");
	TwAddVarRW(tweakbar(), "Shadow Map Size", TW_TYPE_UINT32, &shadowMapSize, "group=light");
	TwAddVarRW(tweakbar(), "Shadow Map Offset", TW_TYPE_FLOAT, &shadowMapOffset, "group=light step=0.0001");
	TwAddVarRW(tweakbar(), "Bloom %", TW_TYPE_FLOAT, &bloomPercentage, "group=postprocess min=0.0 max =1.0 step=0.01");
	TwAddVarRW(tweakbar(), "Exposure", TW_TYPE_FLOAT, &exposureAdjustment, "group=postprocess min=0.0 step=0.1");
	TwAddButton(tweakbar(), "Debug Trace", debugTrace, debugPathTracer.get(), "group=pathtrace");
	TwAddButton(tweakbar(), "Save Trace", saveTrace, debugPathTracer.get(), "group=pathtrace");
	TwAddVarRW(tweakbar(), "Show Debug Image", TW_TYPE_BOOLCPP, &showDebugImage, "group=pathtrace");
	TwAddVarRW(tweakbar(), "Debug Trace Scale", TW_TYPE_FLOAT, &debugTraceScale, "group=pathtrace step=0.01");
	TwAddVarRW(tweakbar(), "SPP", TW_TYPE_UINT32, &samplesPerPixel, "group=pathtrace");
	TwAddVarRW(tweakbar(), "Max Path Depth", TW_TYPE_UINT32, &maxPathDepth, "group=pathtrace");
	TwAddVarRW(tweakbar(), "Clamp Depth", TW_TYPE_UINT32, &clampDepth, "group=pathtrace");
	TwAddVarRW(tweakbar(), "Clamp Radiance", TW_TYPE_FLOAT, &clampRadiance, "group=pathtrace");
	TwAddVarRW(tweakbar(), "Show Lightmap", TW_TYPE_BOOLCPP, &showDebugLightMap, "group=lightmap");
	TwAddVarRW(tweakbar(), "Lightmap Index", TW_TYPE_INT32, &lightMapIndex, "group=lightmap min=0 step=1");
	TwAddVarRW(tweakbar(), "Use Irradiance Map", TW_TYPE_BOOLCPP, &useIrradianceMap, "group=lightmap");
	TwAddVarRW(tweakbar(), "Use AO Map", TW_TYPE_BOOLCPP, &useAOMap, "group=lightmap");
	TwAddButton(tweakbar(), "Make Debug Probe", makeDebugProbe, nullptr, "group=probes");
	TwAddVarRW(tweakbar(), "Probe Mip Level", TW_TYPE_INT32, &debugEnvMapMipLevel, "group=probes min=0");
	TwAddVarRW(tweakbar(), "Show Env Probes", TW_TYPE_BOOLCPP, &showDebugEnvProbes, "group=probes");

	// For setting debug probes
	probeData.camera = getCamera();
	probeData.pipeline = pipeline.get();
	probeData.scene = &scene;
}

void BakedGIApp::render(float elapsedSeconds) {
    if (glm::abs(lastDebugTraceScale - debugTraceScale) > 0.001) {
        debugPathTracer->setDebugImageSize(
            static_cast<int>(getCamera()->getViewportWidth() * debugTraceScale),
            static_cast<int>(getCamera()->getViewportHeight() * debugTraceScale)
        );
        lastDebugTraceScale = debugTraceScale;
    }
    
	debugPathTracer->setSamplesPerPixel(samplesPerPixel);
	debugPathTracer->setMaxPathDepth(maxPathDepth);
	debugPathTracer->setClampDepth(clampDepth);
	debugPathTracer->setClampRadiance(clampRadiance);
	pipeline->setDebugTexture(showDebugImage ? debugPathTracer->getDebugTexture() : nullptr, DebugImageLocation::BottomRight);
	lightMapIndex = std::min(lightMapIndex, (int) scene.getMeshes().size() - 1);
	pipeline->setDebugTexture(showDebugLightMap ? scene.getMeshes()[lightMapIndex].material.lightMap : nullptr, DebugImageLocation::TopRight);
	pipeline->setShadowMapSize(shadowMapSize);
	pipeline->setShadowMapOffset(shadowMapOffset);
	pipeline->setUseIrradianceMap(useIrradianceMap);
	pipeline->setUseAOMap(useAOMap);
	pipeline->setBloomPercentage(bloomPercentage);
	pipeline->setExposureAdjustment(exposureAdjustment);
	pipeline->setDebugEnvMapMipLevel(debugEnvMapMipLevel);
	pipeline->setDebugReflProbeGridEnabled(showDebugEnvProbes);
	scene.render(*pipeline);
}

void BakedGIApp::onResize(int w, int h) {
	glow::glfw::GlfwApp::onResize(w, h);
	pipeline->resizeBuffers(w, h);
}
