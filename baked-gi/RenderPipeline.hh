#pragma once

#include "Mesh.hh"
#include "DirectionalLight.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <vector>

enum class DebugImageLocation {
	TopRight,
	BottomRight
};

class RenderPipeline {
public:
	RenderPipeline();

	void render(const std::vector<Mesh>& meshes);
	void resizeBuffers(int w, int h);

	void setAmbientColor(const glm::vec3& color);
	void attachCamera(const glow::camera::GenericCamera& camera);
	void attachLight(const DirectionalLight& light);

	void setShadowMapSize(int size);
	void setShadowMapOffset(float offset);
	
	void setDebugTexture(const glow::SharedTexture2D& texture, DebugImageLocation location);
	void setUseIrradianceMap(bool use);
	void setUseAOMap(bool use);

private:
	void renderSceneToShadowMap(const std::vector<Mesh>& meshes, const glm::mat4& lightMatrix) const;
	void renderSceneToHDRBuffer(const glm::mat4& lightMatrix) const;

	glow::SharedTextureRectangle hdrColorBuffer;
	glow::SharedTextureRectangle brightnessBuffer;
	glow::SharedTextureRectangle depthBuffer;
	glow::SharedFramebuffer hdrFbo;

	glow::SharedTextureRectangle blurColorBufferA;
	glow::SharedTextureRectangle blurColorBufferB;
	glow::SharedFramebuffer blurFboA;
	glow::SharedFramebuffer blurFboB;

	int shadowMapSize = 4096;
	float shadowMapOffset = 0.001f;
	glow::SharedTextureRectangle shadowBuffer;
	glow::SharedFramebuffer shadowFbo;

	glow::SharedProgram objectShader;
	glow::SharedProgram objectNoTexShader;
	glow::SharedProgram shadowShader;
	glow::SharedProgram skyboxShader;
	glow::SharedProgram downsampleShader;
	glow::SharedProgram blurShader;
	glow::SharedProgram postProcessShader;
	glow::SharedProgram debugImageShader;

	glow::SharedVertexArray vaoQuad;
	glow::SharedVertexArray vaoCube;

	glow::SharedTexture2D topRightDebugTexture;
	glow::SharedTexture2D bottomRightDebugTexture;
	glow::SharedTextureCubeMap skybox;

	const glow::camera::GenericCamera* camera;
	glm::vec3 ambientColor = glm::vec3(0.0f);
	const DirectionalLight* light;
	std::vector<Mesh> texturedMeshes;
	std::vector<Mesh> untexturedMeshes;
	bool useIrradianceMap = true;
	bool useAOMap = true;
};