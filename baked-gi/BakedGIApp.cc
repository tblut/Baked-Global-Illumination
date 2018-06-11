#include "BakedGIApp.hh"

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
#include "tinygltf/tiny_gltf.h"

namespace {
	void debugTrace(void* clientData) {
		PathTracer* tracer = static_cast<PathTracer*>(clientData);
		tracer->traceDebugImage();
	}
}

void BakedGIApp::init() {
	glow::glfw::GlfwApp::init();

	this->setQueryStats(false);

	auto cam = getCamera();
	cam->setPosition({ 0, 0, 1 });
	cam->setTarget({ 0, 0, 0 }, { 0, 1, 0 });

	scene.loadFromGltf(glow::util::pathOf(__FILE__) + "/models/cornellbox_notex.glb");

	pipeline = std::make_unique<RenderPipeline>();
	pipeline->attachCamera(*getCamera());
	pipeline->attachLight(scene.getSun());

	pathTracer = std::make_unique<PathTracer>();
	pathTracer->attachDebugCamera(*getCamera());
	scene.buildPathTracerScene(*pathTracer);

	//TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &ambientColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &scene.getSun().color, "group=light");
	TwAddVarRW(tweakbar(), "Light Power", TW_TYPE_FLOAT, &scene.getSun().power, "group=light");
	TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &scene.getSun().direction, "group=light");
	TwAddButton(tweakbar(), "Debug Trace", debugTrace, pathTracer.get(), "group=pathtrace");
	TwAddVarRW(tweakbar(), "Show Debug Image", TW_TYPE_BOOLCPP, &showDebugImage, "group=pathtrace");
}

void BakedGIApp::render(float elapsedSeconds) {
	pipeline->setDebugTexture(showDebugImage ? pathTracer->getDebugTexture() : nullptr);
	scene.render(*pipeline);
}

void BakedGIApp::onResize(int w, int h) {
	glow::glfw::GlfwApp::onResize(w, h);
	pipeline->resizeBuffers(w, h);
	
	pathTracer->setDebugImageSize(
		getCamera()->getViewportWidth() / 4,
		getCamera()->getViewportHeight() / 4);
}