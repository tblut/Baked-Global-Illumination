#include "RenderPipeline.hh"

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

namespace {
	glm::vec3 gammaToLinear(const glm::vec3& v) {
		return { std::pow(v.x, 2.2f), std::pow(v.y, 2.2f) , std::pow(v.z, 2.2f) };
	}

	glm::vec3 linearToGamma(const glm::vec3& v) {
		return { std::pow(v.x, 1.0f / 2.2f), std::pow(v.y, 1.0f / 2.2f) , std::pow(v.z, 1.0f / 2.2f) };
	}
}

RenderPipeline::RenderPipeline() {
	std::string workDir = glow::util::pathOf(__FILE__);
	objectShader = glow::Program::createFromFile(workDir + "/shaders/Object");
	objectNoTexShader = glow::Program::createFromFile(workDir + "/shaders/ObjectNoTex");
	skyboxShader = glow::Program::createFromFile(workDir + "/shaders/Skybox");
	downsampleShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/Downsample.fsh" });
	blurShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/Blur.fsh" });
	postProcessShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/PostProcess.fsh" });
	debugImageShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/DebugImage.fsh" });
	vaoQuad = glow::geometry::Quad<>().generate();
	vaoHalfQuad = glow::geometry::Quad<>(glow::geometry::Quad<>::attributesOf((glm::vec2*)0), { 0.5, 0.5 }, { 1, 1 }).generate();
	vaoCube = glow::geometry::Cube<>().generate();
	
	hdrColorBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	brightnessBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	depthBuffer = glow::TextureRectangle::create(2, 2, GL_DEPTH_COMPONENT32);
	hdrFbo = glow::Framebuffer::create({ { "fColor", hdrColorBuffer },{ "fBrightColor", brightnessBuffer } }, depthBuffer);

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
}

void RenderPipeline::render(const std::vector<Mesh>& meshes) {
	{ // Render scene to HDR buffer
		auto fbo = hdrFbo->bind();
		const auto& cam = *camera;

		GLOW_SCOPED(enable, GL_DEPTH_TEST);
		GLOW_SCOPED(enable, GL_CULL_FACE);
		GLOW_SCOPED(clearColor, 1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{ // Render objects
			for (const auto& mesh : meshes) {
				if (mesh.material.colorMap) {
					auto p = objectShader->use();
					p.setUniform("uView", cam.getViewMatrix());
					p.setUniform("uProj", cam.getProjectionMatrix());
					p.setUniform("uModel", mesh.transform);
					p.setUniform("uNormalMat", glm::transpose(glm::inverse(glm::mat3(mesh.transform))));
					p.setUniform("uCamPos", cam.getPosition());
					p.setUniform("uAmbientColor", gammaToLinear(ambientColor));
					p.setUniform("uLightDir", glm::normalize(-light->direction));
					p.setUniform("uLightColor", gammaToLinear(light->color) * light->power);

					p.setUniform("uBaseColor", gammaToLinear(mesh.material.baseColor));
					p.setUniform("uMetallic", mesh.material.metallic);
					p.setUniform("uRoughness", mesh.material.roughness);
					p.setTexture("uTextureColor", mesh.material.colorMap);
					p.setTexture("uTextureRoughness", mesh.material.roughnessMap);
					p.setTexture("uTextureNormal", mesh.material.normalMap);

					mesh.vao->bind().draw();
				}
				else {
					auto p = objectNoTexShader->use();
					p.setUniform("uView", cam.getViewMatrix());
					p.setUniform("uProj", cam.getProjectionMatrix());
					p.setUniform("uModel", mesh.transform);
					p.setUniform("uNormalMat", glm::transpose(glm::inverse(glm::mat3(mesh.transform))));
					p.setUniform("uCamPos", cam.getPosition());
					p.setUniform("uAmbientColor", gammaToLinear(ambientColor));
					p.setUniform("uLightDir", glm::normalize(-light->direction));
					p.setUniform("uLightColor", gammaToLinear(light->color) * light->power);

					p.setUniform("uBaseColor", gammaToLinear(mesh.material.baseColor));
					p.setUniform("uMetallic", mesh.material.metallic);
					p.setUniform("uRoughness", mesh.material.roughness);

					mesh.vao->bind().draw();
				}
			}
		}

		{ // Render skybox
			GLOW_SCOPED(depthMask, GL_FALSE);
			GLOW_SCOPED(disable, GL_CULL_FACE);
			GLOW_SCOPED(depthFunc, GL_LEQUAL);

			glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(cam.getViewMatrix()));

			auto p = skyboxShader->use();
			p.setUniform("uView", viewNoTranslation);
			p.setUniform("uProj", cam.getProjectionMatrix());
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

	// Debug texture
	if (debugTexture) {
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		auto p = debugImageShader->use();
		p.setUniform("uViewportSize", glm::vec2(camera->getViewportSize()));
		p.setTexture("uDebugImage", debugTexture);
		vaoHalfQuad->bind().draw();
	}
}

void RenderPipeline::resizeBuffers(int w, int h) {
	hdrColorBuffer->bind().resize(w, h);
	brightnessBuffer->bind().resize(w, h);
	depthBuffer->bind().resize(w, h);
	blurColorBufferA->bind().resize(w / 2, h / 2);
	blurColorBufferB->bind().resize(w / 2, h / 2);
}

void RenderPipeline::setAmbientColor(const glm::vec3& color) {
	ambientColor = color;
}

void RenderPipeline::attachCamera(const glow::camera::GenericCamera& camera) {
	this->camera = &camera;
}

void RenderPipeline::attachLight(const DirectionalLight& light) {
	this->light = &light;
}

void RenderPipeline::setDebugTexture(glow::SharedTexture2D texture) {
	this->debugTexture = texture;
}