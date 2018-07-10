#include "RenderPipeline.hh"
#include "ColorUtils.hh"
#include "Scene.hh"

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

RenderPipeline::RenderPipeline() {
	std::string workDir = glow::util::pathOf(__FILE__);
	objectShader = glow::Program::createFromFiles({ workDir + "/shaders/Object.vsh", workDir + "/shaders/Object.fsh" });
	objectTexShader = glow::Program::createFromFiles({ workDir + "/shaders/ObjectTex.vsh", workDir + "/shaders/ObjectTex.fsh" });
    objectIBLShader = glow::Program::createFromFiles({ workDir + "/shaders/Object.vsh", workDir + "/shaders/ObjectIBL.fsh" });
    objectTexIBLShader = glow::Program::createFromFiles({ workDir + "/shaders/ObjectTex.vsh", workDir + "/shaders/ObjectTexIBL.fsh" });
	shadowShader = glow::Program::createFromFile(workDir + "/shaders/Shadow");
	skyboxShader = glow::Program::createFromFile(workDir + "/shaders/Skybox");
	downsampleShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/Downsample.fsh" });
	blurShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/Blur.fsh" });
	postProcessShader = glow::Program::createFromFiles({ workDir + "/shaders/Fullscreen.vsh", workDir + "/shaders/PostProcess.fsh" });
	debugImageShader = glow::Program::createFromFile(workDir + "/shaders/DebugImage");
	debugEnvMapShader = glow::Program::createFromFile(workDir + "/shaders/DebugEnvMap");
	precalcEnvBrdfLutShader = glow::Program::createFromFile(workDir + "/shaders/PrecalcEnvBrdfLut.csh");
	precalcEnvMapShader = glow::Program::createFromFile(workDir + "/shaders/PrecalcEnvMap.csh");

	vaoQuad = glow::geometry::Quad<>().generate();
	vaoCube = glow::geometry::Cube<>().generate();
	vaoSphere = glow::geometry::UVSphere<>().generate();
	
	hdrColorBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	brightnessBuffer = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	depthBuffer = glow::TextureRectangle::create(2, 2, GL_DEPTH_COMPONENT32);
	hdrFbo = glow::Framebuffer::create({ { "fColor", hdrColorBuffer },{ "fBrightColor", brightnessBuffer } }, depthBuffer);

	blurColorBufferA = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	blurColorBufferB = glow::TextureRectangle::create(2, 2, GL_RGB16F);
	blurFboA = glow::Framebuffer::create({ { "fColor", blurColorBufferA } });
	blurFboB = glow::Framebuffer::create({ { "fColor", blurColorBufferB } });

	shadowBuffer = glow::TextureRectangle::create(shadowMapSize, shadowMapSize, GL_DEPTH_COMPONENT32);
	shadowFbo = glow::Framebuffer::create(std::vector<glow::Framebuffer::Attachment>(), shadowBuffer);
	shadowBuffer->bind().setFilter(GL_LINEAR, GL_LINEAR);
	shadowBuffer->bind().setCompareFunc(GL_LEQUAL);
	shadowBuffer->bind().setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
	shadowBuffer->bind().setWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
	shadowBuffer->bind().setBorderColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	auto pbt = workDir + "/textures/miramar";
	skybox = glow::TextureCubeMap::createFromData(glow::TextureData::createFromFileCube(
		pbt + "/posx.jpg",
		pbt + "/negx.jpg",
		pbt + "/posy.jpg",
		pbt + "/negy.jpg",
		pbt + "/posz.jpg",
		pbt + "/negz.jpg",
		glow::ColorSpace::sRGB));

	envLutGGX = computeEnvLutGGX(64, 64);

	auto tex = glow::TextureCubeMap::createStorageImmutable(512, 512, GL_RGBA16F);
	{
		auto t = tex->bind();
		t.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		t.setMagFilter(GL_LINEAR);
		t.generateMipmaps();
	}
	defaultEnvMapGGX = tex;

	this->defaultEnvMapGGX = computeEnvMapGGX(skybox, 512);
}

void RenderPipeline::render(const std::vector<Mesh>& meshes) {
	fillRenderQueues(meshes);

	const auto& cam = *camera;
	auto lightMatrix = makeLightMatrix(cam.getPosition());

	renderSceneToShadowMap(meshes, lightMatrix);
	renderSceneToFBO(hdrFbo, cam, lightMatrix);

    // Render debug env map
	if (debugEnvMap) {
		GLOW_SCOPED(enable, GL_DEPTH_TEST);
		GLOW_SCOPED(enable, GL_CULL_FACE);

		auto fbo = hdrFbo->bind();
		auto p = debugEnvMapShader->use();
		p.setUniform("uView", cam.getViewMatrix());
		p.setUniform("uProj", cam.getProjectionMatrix());
		p.setUniform("uModel", glm::translate(debugEnvMapPosition));
		p.setTexture("uEnvMap", debugEnvMap);
		p.setUniform("uMipLevel", static_cast<float>(debugEnvMapMipLevel));
		vaoSphere->bind().draw();
	}

	// Render debug probe grid
	if (isDebugProbeGridEnabled) {
        GLOW_SCOPED(enable, GL_DEPTH_TEST);
		GLOW_SCOPED(enable, GL_CULL_FACE);

		auto fbo = hdrFbo->bind();
		auto p = debugEnvMapShader->use();
		p.setUniform("uView", cam.getViewMatrix());
		p.setUniform("uProj", cam.getProjectionMatrix());
        
        for (const auto& probe : reflectionProbes) {
            p.setUniform("uModel", glm::translate(probe.position) * glm::scale(glm::vec3(0.25f)));
            p.setTexture("uEnvMap", probe.ggxEnvMap);
            p.setUniform("uMipLevel", static_cast<float>(debugEnvMapMipLevel));
            vaoSphere->bind().draw();
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

		for (float offset : {0.0f, 1.0f, 2.0f, 2.0f, 3.0f}) {
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
		p.setUniform("uExposureAdjustment", exposureAdjustment);
		p.setTexture("uHdrBuffer", hdrColorBuffer);
		p.setTexture("uBloomBuffer", blurColorBufferB);
		vaoQuad->bind().draw();
	}

	// Debug texture
	if (topRightDebugTexture || bottomRightDebugTexture) {
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		auto p = debugImageShader->use();

		if (topRightDebugTexture) {
			p.setUniform("uOffset", glm::vec2(0.5f, 0.5f));
			p.setUniform("uScale", glm::vec2(0.5f, 0.5f));
			p.setTexture("uDebugImage", topRightDebugTexture);
			vaoQuad->bind().draw();
		}

		if (bottomRightDebugTexture) {
			p.setUniform("uOffset", glm::vec2(0.5f, 0.0f));
			p.setUniform("uScale", glm::vec2(0.5f, 0.5f));
			p.setTexture("uDebugImage", bottomRightDebugTexture);
			vaoQuad->bind().draw();
		}
	}
}

void RenderPipeline::resizeBuffers(int w, int h) {
	hdrColorBuffer->bind().resize(w, h);
	brightnessBuffer->bind().resize(w, h);
	depthBuffer->bind().resize(w, h);
	blurColorBufferA->bind().resize(w / 2, h / 2);
	blurColorBufferB->bind().resize(w / 2, h / 2);
}

glow::SharedTextureCubeMap RenderPipeline::renderEnvironmentMap(const glm::vec3& position, int size, const std::vector<Mesh>& meshes) {
	auto lightMatrix = makeLightMatrix(position);
	renderSceneToShadowMap(meshes, lightMatrix);

	fillRenderQueues(meshes);

	auto envMap = glow::TextureCubeMap::createStorageImmutable(size, size, GL_RGBA16F);
	auto envDepth = glow::Texture2D::createStorageImmutable(size, size, GL_DEPTH_COMPONENT32);
	auto envMapFbo = glow::Framebuffer::createDepthOnly(envDepth);

	GLOW_SCOPED(enable, GL_DEPTH_TEST);
	GLOW_SCOPED(enable, GL_CULL_FACE);
	GLOW_SCOPED(clearColor, 0, 0, 0, 0);

	/*
	GL_TEXTURE_CUBE_MAP_POSITIVE_X
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	*/
	const glm::vec3 faceDirVectors[] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f)
	};

	const glm::vec3 faceUpVectors[] = {
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f)
	};

	glow::camera::GenericCamera envMapCam;
	envMapCam.setViewportSize(size, size);
	envMapCam.setAspectRatio(size / static_cast<float>(size));
	envMapCam.setVerticalFieldOfView(90.0f);
	envMapCam.setPosition(position);
/*
	auto tex = glow::TextureCubeMap::createStorageImmutable(512, 512, GL_RGBA16F);
	{
		auto t = tex->bind();
		t.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		t.setMagFilter(GL_LINEAR);
		t.generateMipmaps();
	}
	envMapGGX = tex;
*/
	for (int faceID = 0; faceID < 6; ++faceID) {
		envMapFbo->bind().attachColor("fColor", envMap, 0, faceID);
		envMapCam.setLookAtMatrix(position, position + faceDirVectors[faceID], faceUpVectors[faceID]);
		renderSceneToFBO(envMapFbo, envMapCam, lightMatrix);
	}

	
	this->defaultEnvMapGGX = computeEnvMapGGX(envMap, size);

	return defaultEnvMapGGX;
}

void RenderPipeline::setReflectionProbes(const std::vector<ReflectionProbe>& probes) {
    reflectionProbes = probes;
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

void RenderPipeline::setShadowMapSize(int size) {
	shadowMapSize = size;
}

void RenderPipeline::setShadowMapOffset(float offset) {
	shadowMapOffset = offset;
}

void RenderPipeline::setDebugTexture(const glow::SharedTexture2D& texture, DebugImageLocation location) {
	if (location == DebugImageLocation::TopRight) {
		this->topRightDebugTexture = texture;
	}
	else {
		this->bottomRightDebugTexture = texture;
	}
}

void RenderPipeline::setDebugEnvMap(const glow::SharedTextureCubeMap& cubeMap, const glm::vec3& position) {
	debugEnvMap = cubeMap;
	debugEnvMapPosition = position;
}

void RenderPipeline::setDebugEnvMapMipLevel(int value) {
	debugEnvMapMipLevel = value;
}

void RenderPipeline::makeDebugReflProbeGrid(const Scene& scene, int width, int height, int depth) {
    glm::vec3 min, max;
    scene.getBoundingBox(min, max);
  
    min.y += 1.0f;
    max.y += 1.0f;
    glm::vec3 stepSize = (max - min) / glm::vec3(width, height, depth);
    
    min += stepSize * 0.2f;
    max -= stepSize * 0.2f;
    stepSize = (max - min) / glm::vec3(width, height, depth);
    
    for (int z = 0; z <= depth; ++z) {
        for (int y = 0; y <= height; ++y) {
            for (int x = 0; x <= width; ++x) {
                glm::vec3 pos = min + stepSize * glm::vec3(x, y ,z);
                probeGrid.push_back(renderEnvironmentMap(pos, 256, scene.getMeshes()));
                probeGridPositions.push_back(pos);
            }
        } 
    }
}

void RenderPipeline::setDebugReflProbeGridEnabled(bool enabled) {
    isDebugProbeGridEnabled = enabled;
}

void RenderPipeline::setUseIrradianceMap(bool use) {
	useIrradianceMap = use;
}

void RenderPipeline::setUseAOMap(bool use) {
	useAOMap = use;
}

void RenderPipeline::setBloomPercentage(float value) {
	bloomPercentage = value;
}

void RenderPipeline::setExposureAdjustment(float value) {
	exposureAdjustment = value;
}

void RenderPipeline::setProbes(const glm::vec3& pos, const glm::vec3& halfExtents) {
    probePos = pos;
    probeAabbMin = probePos - halfExtents;
    probeAabbMax = probePos + halfExtents;
    
    /*
    probePos[0] = pos + glm::vec3(-halfExtents.x, -halfExtents.y, halfExtents.z);
    probePos[1] = pos + glm::vec3(halfExtents.x, -halfExtents.y, halfExtents.z);
    probePos[2] = pos + glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z);
    probePos[3] = pos + glm::vec3(-halfExtents.x, halfExtents.y, halfExtents.z);
    probePos[4] = pos + glm::vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z);
    probePos[5] = pos + glm::vec3(halfExtents.x, -halfExtents.y, -halfExtents.z);
    probePos[6] = pos + glm::vec3(halfExtents.x, halfExtents.y, -halfExtents.z);
    probePos[7] = pos + glm::vec3(-halfExtents.x, halfExtents.y, -halfExtents.z);
    
    for (int i = 0; i < 8; ++i) {
        probeAabbMin[i] = probePos[i] - halfExtents;
        probeAabbMax[i] = probePos[i] + halfExtents;
    }
    */
}

void RenderPipeline::renderSceneToShadowMap(const std::vector<Mesh>& meshes, const glm::mat4& lightMatrix) const {
	if (shadowBuffer->getWidth() != shadowMapSize) {
		shadowBuffer->bind().resize(shadowMapSize, shadowMapSize);
	}

	auto fbo = shadowFbo->bind();

	GLOW_SCOPED(viewport, 0, 0, shadowMapSize, shadowMapSize);
	GLOW_SCOPED(enable, GL_DEPTH_TEST);
	GLOW_SCOPED(enable, GL_CULL_FACE);
	GLOW_SCOPED(depthFunc, GL_LESS);
	glClear(GL_DEPTH_BUFFER_BIT);

	auto p = shadowShader->use();
	p.setUniform("uViewProj", lightMatrix);

	for (const auto& mesh : meshes) {
		p.setUniform("uModel", mesh.transform);
		mesh.vao->bind().draw();
	}
}

void RenderPipeline::renderSceneToFBO(const glow::SharedFramebuffer& targetFbo, const glow::camera::GenericCamera& cam,
									  const glm::mat4& lightMatrix) const {
	auto fbo = targetFbo->bind();

	GLOW_SCOPED(enable, GL_DEPTH_TEST);
	GLOW_SCOPED(enable, GL_CULL_FACE);
	GLOW_SCOPED(clearColor, 1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!texturedMeshes.empty()) { // Render textured objects
		auto p = objectTexIBLShader->use();
		p.setUniform("uView", cam.getViewMatrix());
		p.setUniform("uProj", cam.getProjectionMatrix());
		p.setUniform("uCamPos", cam.getPosition());
		p.setUniform("uAmbientColor", gammaToLinear(ambientColor));
		p.setUniform("uLightDir", glm::normalize(-light->direction));
		p.setUniform("uLightColor", gammaToLinear(light->color) * light->power);
		p.setUniform("uShadowMapSize", glm::vec2(static_cast<float>(shadowMapSize)));
		p.setUniform("uShadowOffset", shadowMapOffset);
		p.setUniform("uLightMatrix", lightMatrix);
		p.setUniform("uUseIrradianceMap", useIrradianceMap);
		p.setUniform("uUseAOMap", useAOMap);
		p.setUniform("uBloomPercentage", bloomPercentage);
		p.setTexture("uTextureShadow", shadowBuffer);
		p.setTexture("uEnvMapGGX", defaultEnvMapGGX);
		p.setTexture("uEnvLutGGX", envLutGGX);

		for (const auto& mesh : texturedMeshes) {
			p.setUniform("uModel", mesh.transform);
			p.setUniform("uNormalMat", glm::transpose(glm::inverse(glm::mat3(mesh.transform))));
			p.setUniform("uBaseColor", gammaToLinear(mesh.material.baseColor));
			p.setUniform("uMetallic", mesh.material.metallic);
			p.setUniform("uRoughness", mesh.material.roughness);
			p.setTexture("uTextureColor", mesh.material.colorMap);
			p.setTexture("uTextureRoughness", mesh.material.roughnessMap);
			p.setTexture("uTextureNormal", mesh.material.normalMap);
			p.setTexture("uTextureIrradiance", mesh.material.lightMap);
			p.setTexture("uTextureAO", mesh.material.aoMap);

			mesh.vao->bind().draw();
		}
	}

	if (!untexturedMeshes.empty()) { // Render untextured objects
		auto p = objectIBLShader->use();
		p.setUniform("uView", cam.getViewMatrix());
		p.setUniform("uProj", cam.getProjectionMatrix());
		p.setUniform("uCamPos", cam.getPosition());
		p.setUniform("uAmbientColor", gammaToLinear(ambientColor));
		p.setUniform("uLightDir", glm::normalize(-light->direction));
		p.setUniform("uLightColor", gammaToLinear(light->color) * light->power);
		p.setUniform("uShadowMapSize", glm::vec2(static_cast<float>(shadowMapSize)));
		p.setUniform("uShadowOffset", shadowMapOffset);
		p.setUniform("uLightMatrix", lightMatrix);
		p.setUniform("uUseIrradianceMap", useIrradianceMap);
		p.setUniform("uUseAOMap", useAOMap);
		p.setUniform("uBloomPercentage", bloomPercentage);
        p.setUniform("uProbePos", probePos);
        p.setUniform("uAABBMin", probeAabbMin);
        p.setUniform("uAABBMax", probeAabbMax);
		p.setTexture("uTextureShadow", shadowBuffer);
		p.setTexture("uEnvMapGGX", defaultEnvMapGGX);
		p.setTexture("uEnvLutGGX", envLutGGX);

		for (const auto& mesh : untexturedMeshes) {
			p.setUniform("uModel", mesh.transform);
			p.setUniform("uNormalMat", glm::transpose(glm::inverse(glm::mat3(mesh.transform))));
			p.setUniform("uBaseColor", gammaToLinear(mesh.material.baseColor));
			p.setUniform("uMetallic", mesh.material.metallic);
			p.setUniform("uRoughness", mesh.material.roughness);
			p.setTexture("uTextureIrradiance", mesh.material.lightMap);
			p.setTexture("uTextureAO", mesh.material.aoMap);

			mesh.vao->bind().draw();
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
		p.setUniform("uBloomPercentage", bloomPercentage);
		p.setTexture("uSkybox", skybox);

		vaoCube->bind().draw();
	}
}

void RenderPipeline::fillRenderQueues(const std::vector<Mesh>& meshes) {
	texturedMeshes.clear();
	untexturedMeshes.clear();
	for (const auto& mesh : meshes) {
		if (mesh.material.colorMap) {
			texturedMeshes.push_back(mesh);
		}
		else {
			untexturedMeshes.push_back(mesh);
		}
	}
}

glm::mat4 RenderPipeline::makeLightMatrix(const glm::vec3& camPos) const {
	auto lightViewMatrix = glm::lookAt(camPos - light->direction * 50.0f, camPos, glm::vec3(0, 1, 0));
	auto lightProjMatrix = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 150.0f);
	auto lightMatrix = lightProjMatrix * lightViewMatrix;
	return lightMatrix;
}

glow::SharedTexture2D RenderPipeline::computeEnvLutGGX(int width, int height) const {
	auto tex = glow::Texture2D::createStorageImmutable(width, height, GL_RG16F, 1);
	{
		auto t = tex->bind();
		t.setMinFilter(GL_LINEAR);
		t.setMagFilter(GL_LINEAR);
		t.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	const int localSize = 4;
	{
		auto p = precalcEnvBrdfLutShader->use();
		p.setImage(0, tex, GL_WRITE_ONLY);
		p.compute((width - 1) / localSize + 1, (height - 1) / localSize + 1);
	}

	return tex;
}

glow::SharedTextureCubeMap RenderPipeline::computeEnvMapGGX(const glow::SharedTextureCubeMap& envMap, int size) const {
	auto tex = glow::TextureCubeMap::createStorageImmutable(size, size, GL_RGBA16F);
	{
		auto t = tex->bind();
		t.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
		t.setMagFilter(GL_LINEAR);
	}

	const int localSize = 4;
	{
		auto p = precalcEnvMapShader->use();
		p.setTexture("uEnvMap", envMap);
		int miplevel = 0;
		int maxLevel = static_cast<int>(glm::floor(glm::log2(static_cast<float>(size))));
		for (int tsize = size; tsize > 0; tsize /= 2) {
			auto roughness = miplevel / (float)maxLevel;
			p.setUniform("uRoughness", roughness);
			p.setImage(0, tex, GL_WRITE_ONLY, miplevel);
			p.compute((tsize - 1) / localSize + 1, (tsize - 1) / localSize + 1, 6);
			++miplevel;
		}
	}

	tex->setMipmapsGenerated(true);
	return tex;
}
