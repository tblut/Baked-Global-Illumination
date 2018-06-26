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

	glow::SharedTextureCubeMap renderEnvironmentMap(const glm::vec3& position, int size, const std::vector<Mesh>& meshes);

	void setAmbientColor(const glm::vec3& color);
	void attachCamera(const glow::camera::GenericCamera& camera);
	void attachLight(const DirectionalLight& light);

	void setShadowMapSize(int size);
	void setShadowMapOffset(float offset);
	
	void setDebugTexture(const glow::SharedTexture2D& texture, DebugImageLocation location);
	void setDebugEnvMap(const glow::SharedTextureCubeMap& cubeMap, const glm::vec3& position = glm::vec3(0.0));
	void setDebugEnvMapMipLevel(int value);
    
	void setUseIrradianceMap(bool use);
	void setUseAOMap(bool use);
	void setBloomPercentage(float value);
	void setExposureAdjustment(float value);
    
    void setProbe(const glm::vec3& pos, const glm::vec3& halfExtents);

private:
	void renderSceneToShadowMap(const std::vector<Mesh>& meshes, const glm::mat4& lightMatrix) const;
	void renderSceneToFBO(const glow::SharedFramebuffer& targetFbo,
						  const glow::camera::GenericCamera& cam,
						  const glm::mat4& lightMatrix) const;
	void fillRenderQueues(const std::vector<Mesh>& meshes);
	glm::mat4 makeLightMatrix(const glm::vec3& camPos) const;
	glow::SharedTexture2D computeEnvLutGGX(int width, int height) const;
	glow::SharedTextureCubeMap computeEnvMapGGX(const glow::SharedTextureCubeMap& envMap, int size) const;

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
	glow::SharedProgram debugEnvMapShader;
	glow::SharedProgram precalcEnvBrdfLutShader;
	glow::SharedProgram precalcEnvMapShader;

	glow::SharedVertexArray vaoQuad;
	glow::SharedVertexArray vaoCube;
	glow::SharedVertexArray vaoSphere;

	glow::SharedTexture2D topRightDebugTexture;
	glow::SharedTexture2D bottomRightDebugTexture;
	glow::SharedTextureCubeMap debugEnvMap;
	glow::SharedTextureCubeMap skybox;
	glow::SharedTexture2D envLutGGX;
	glow::SharedTextureCubeMap envMapGGX;

	const glow::camera::GenericCamera* camera;
	glm::vec3 ambientColor = glm::vec3(0.0f);
	const DirectionalLight* light;
	std::vector<Mesh> texturedMeshes;
	std::vector<Mesh> untexturedMeshes;
	bool useIrradianceMap = true;
	bool useAOMap = true;
	float bloomPercentage = 0.02f;
	float exposureAdjustment = 1.0f;
	glm::vec3 debugEnvMapPosition;
	int debugEnvMapMipLevel = 0;
    
    glm::vec3 probePos;
    glm::vec3 probeAabbMin = glm::vec3(-5.0f);
    glm::vec3 probeAabbMax = glm::vec3(5.0f);
};
