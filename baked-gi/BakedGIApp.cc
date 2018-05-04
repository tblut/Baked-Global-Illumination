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

void BakedGIApp::init() {
	glow::glfw::GlfwApp::init();

	std::string workDir = glow::util::pathOf(__FILE__);
	objectShader = glow::Program::createFromFile(workDir + "/shaders/object");
	skyboxShader = glow::Program::createFromFile(workDir + "/shaders/skybox");
	downsampleShader = glow::Program::createFromFiles({ workDir + "/shaders/fullscreen.vsh", workDir + "/shaders/downsample.fsh" });
	blurShader = glow::Program::createFromFiles({ workDir + "/shaders/fullscreen.vsh", workDir + "/shaders/blur.fsh" });
	postProcessShader = glow::Program::createFromFiles({ workDir + "/shaders/fullscreen.vsh", workDir + "/shaders/postprocess.fsh" });
	vertexArray = glow::geometry::UVSphere<>().generate();
	vaoQuad = glow::geometry::Quad<>().generate();
	vaoCube = glow::geometry::Cube<>().generate();
	textureColor = glow::Texture2D::createFromFile(workDir + "/textures/panels.color.png", glow::ColorSpace::sRGB);
	textureNormal = glow::Texture2D::createFromFile(workDir + "/textures/panels.normal.png", glow::ColorSpace::Linear);

	hdrColorBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	brightnessBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	depthBuffer = glow::TextureRectangle::create(2, 2, GL_DEPTH_COMPONENT32);
	hdrFbo = glow::Framebuffer::create({ { "fColor", hdrColorBuffer }, { "fBrightColor", brightnessBuffer } }, depthBuffer);

	blurColorBufferA = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	blurColorBufferB = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	blurFboA = glow::Framebuffer::create({ { "fColor", blurColorBufferA } });
	blurFboB = glow::Framebuffer::create({ { "fColor", blurColorBufferB } });

	auto pbt = workDir + "/textures/miramar";
	skybox = glow::TextureCubeMap::createFromData(glow::TextureData::createFromFileCube(
		pbt + "/posx.jpg",
		pbt + "/negx.jpg",
		pbt + "/posy.jpg",
		pbt + "/negy.jpg",
		pbt + "/posz.jpg",
		pbt + "/negz.jpg",
		glow::ColorSpace::sRGB));

	//vertexArray = glow::assimp::Importer().load(glow::util::pathOf(__FILE__) + "/models/kitchen/Country-Kitchen.fbx");

	auto cam = getCamera();
	cam->setPosition({ 2, 2, 2 });
	cam->setTarget({ 0, 0, 0 }, { 0, 1, 0 });

	TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &ambientColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &lightColor, "group=light");
	TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &lightDir, "group=light");
	TwAddVarRW(tweakbar(), "Base Color", TW_TYPE_COLOR3F, &baseColor, "group=shading");
	TwAddVarRW(tweakbar(), "Roughness", TW_TYPE_FLOAT, &roughness, "group=shading step=0.01 min=0.01 max=1.0");
	TwAddVarRW(tweakbar(), "Metallic", TW_TYPE_FLOAT, &metallic, "group=shading step=0.01 min=0.00 max=1.0");

	//model.loadFromFile(glow::util::pathOf(__FILE__) + "/models/kitchen/Country-Kitchen.obj", glow::util::pathOf(__FILE__) + "/models/kitchen/Textures/");
	model.loadFromFile(glow::util::pathOf(__FILE__) + "/models/Forge.fbx", glow::util::pathOf(__FILE__) + "/models/3DForge/Fantasy_Interiors/Villages_&_Towns/Textures/");
}

void BakedGIApp::render(float elapsedSeconds) {
	{ // Render scene to HDR buffer
		auto fbo = hdrFbo->bind();
		auto cam = getCamera();
		
		GLOW_SCOPED(enable, GL_DEPTH_TEST);
		GLOW_SCOPED(enable, GL_CULL_FACE);
		GLOW_SCOPED(clearColor, 0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{ // Render objects
			auto p = objectShader->use();
			p.setUniform("uView", cam->getViewMatrix());
			p.setUniform("uProj", cam->getProjectionMatrix());
			p.setUniform("uModel", glm::mat4(1.0f));
			p.setUniform("uCamPos", cam->getPosition());
			p.setUniform("uBaseColor", baseColor);
			p.setUniform("uMetallic", metallic);
			p.setUniform("uRoughness", roughness);
			p.setUniform("uAmbientColor", ambientColor);
			p.setUniform("uLightDir", lightDir);
			p.setUniform("uLightColor", lightColor);
			p.setTexture("uTextureColor", textureColor);
			p.setTexture("uTextureNormal", textureNormal);

			//vertexArray->bind().draw();
			model.render(p);
		}

		{ // Render skybox
			GLOW_SCOPED(depthMask, GL_FALSE);
			GLOW_SCOPED(disable, GL_CULL_FACE);
			GLOW_SCOPED(depthFunc, GL_LEQUAL);

			glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(cam->getViewMatrix()));

			auto p = skyboxShader->use();
			p.setUniform("uView", viewNoTranslation);
			p.setUniform("uProj", cam->getProjectionMatrix());
			p.setTexture("uSkybox", skybox);

			vaoCube->bind().draw();
		}
	}

	{ // Bloom
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		{ // Downsample
			auto fbo = blurFboA->bind();
			auto p = downsampleShader->use();
			p.setTexture("uColorBuffer", brightnessBuffer);
			vaoQuad->bind().draw();
		}

		// Kawase blur
		int currentFbo = 1;
		glow::SharedFramebuffer* blurFbos[2] = { &blurFboA, &blurFboB };
		glow::SharedTextureRectangle* blurTextures[2] = { &blurColorBufferA, &blurColorBufferB };

		auto p = blurShader->use();

		for (float offset : {0, 1, 2, 2, 3}) {
			auto fbo = (*blurFbos[currentFbo])->bind();
			currentFbo = (currentFbo + 1) % 2;

			p.setUniform("uOffset", offset);
			p.setTexture("uColorBuffer", *blurTextures[currentFbo]);
			vaoQuad->bind().draw();
		}
	}

	{ // Post-processing
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		auto p = postProcessShader->use();
		p.setTexture("uHdrBuffer", hdrColorBuffer);
		p.setTexture("uBloomBuffer", blurColorBufferB);
		vaoQuad->bind().draw();
	}
}

void BakedGIApp::onResize(int w, int h) {
	glow::glfw::GlfwApp::onResize(w, h);
	hdrColorBuffer->bind().resize(w, h);
	brightnessBuffer->bind().resize(w, h);
	depthBuffer->bind().resize(w, h);
	blurColorBufferA->bind().resize(w / 2, h / 2);
	blurColorBufferB->bind().resize(w / 2, h / 2);
}