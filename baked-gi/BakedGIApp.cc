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
#include <AntTweakBar.h>

#include <embree3/rtcore.h>
#include "tinygltf/tiny_gltf.h"

void BakedGIApp::init() {
	glow::glfw::GlfwApp::init();

	auto cam = getCamera();
	cam->setPosition({ 2, 2, 2 });
	cam->setTarget({ 0, 0, 0 }, { 0, 1, 0 });

	TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &ambientColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &lightColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &lightDir, "group=light");

	pipeline = std::make_unique<RenderPipeline>();

	//model.loadFromFile(glow::util::pathOf(__FILE__) + "/models/kitchen/Country-Kitchen.obj", glow::util::pathOf(__FILE__) + "/models/kitchen/Textures/");
	//model.loadFromFile(glow::util::pathOf(__FILE__) + "/models/living_room/living_room.obj", glow::util::pathOf(__FILE__) + "/models/living_room/textures/");
	scene.loadFromGltf(glow::util::pathOf(__FILE__) + "/models/cornellbox.glb");
}

void BakedGIApp::render(float elapsedSeconds) {
	pipeline->setAmbientColor(ambientColor);
	pipeline->setLightColor(lightColor);
	pipeline->setLightDirection(lightDir);
	scene.render(*getCamera(), *pipeline);
}

void BakedGIApp::onResize(int w, int h) {
	glow::glfw::GlfwApp::onResize(w, h);
	pipeline->resizeBuffers(w, h);
}