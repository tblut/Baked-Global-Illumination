#pragma once

#include "Mesh.hh"
#include "DirectionalLight.hh"
#include "ReflectionProbe.hh"
#include "VoxelGrid.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <vector>

class Scene;

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
	void bakeReflectionProbes(const std::vector<ReflectionProbe>& probes, int size, int bounces, const std::vector<Mesh>& meshes);

	void setProbeVisibilityGrid(const VoxelGrid<glm::ivec3>& grid);
    void setReflectionProbes(const std::vector<ReflectionProbe>& probes);
	void setAmbientColor(const glm::vec3& color);
	void attachCamera(const glow::camera::GenericCamera& camera);
	void attachLight(const DirectionalLight& light);

	void setShadowMapSize(int size);
	void setShadowMapOffset(float offset);
	
	void setDebugTexture(const glow::SharedTexture2D& texture, DebugImageLocation location);
	void setDebugEnvMap(const glow::SharedTextureCubeMap& cubeMap, const glm::vec3& position = glm::vec3(0.0));
	void setDebugEnvMapMipLevel(int value);
    
    void setDebugReflProbeGridEnabled(bool enabled);
	void setShowDebugProbeVisGrid(bool show);
	void setCurrentProbeIndex(int index);
	void setProbePlancementPreview(bool enabled, glm::vec3 position = glm::vec3(0));

	void setUseIrradianceMap(bool use);
	void setUseAOMap(bool use);
	void setUseIBL(bool use);
	void setUseLocalProbes(bool use);
	void setBloomPercentage(float value);
	void setExposureAdjustment(float value);

private:
	void renderSceneToShadowMap(const std::vector<Mesh>& meshes, const glm::mat4& lightMatrix) const;
	void renderSceneToFBO(const glow::SharedFramebuffer& targetFbo,
						  const glow::camera::GenericCamera& cam,
						  const glm::mat4& lightMatrix) const;
	void fillRenderQueues(const std::vector<Mesh>& meshes);
	glm::mat4 makeLightMatrix(const glm::vec3& camPos) const;
	glow::SharedTexture2D computeEnvLutGGX(int width, int height) const;
	glow::SharedTextureCubeMap computeEnvMapGGX(const glow::SharedTextureCubeMap& envMap, int size) const;
	void computeEnvMapGGXProbe(int layer, int size, const glow::SharedTextureCubeMapArray& sourceArray,
		const glow::SharedTextureCubeMapArray& targetArray) const;
	glow::SharedTextureCubeMap makeBlackCubeMap(int size) const;
	glow::SharedTextureCubeMapArray makeDefaultReflectionProbes(int size) const;
	void renderDebugGrid(glm::vec3 min, glm::vec3 max, glm::vec3 voxelSize, glm::ivec3 gridDimensions, const glm::vec3& color) const;

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
    glow::SharedProgram objectTexShader;
    glow::SharedProgram objectIBLShader;
    glow::SharedProgram objectTexIBLShader;
	glow::SharedProgram shadowShader;
	glow::SharedProgram skyboxShader;
	glow::SharedProgram downsampleShader;
	glow::SharedProgram blurShader;
	glow::SharedProgram postProcessShader;
	glow::SharedProgram debugImageShader;
	glow::SharedProgram debugEnvMapShader;
	glow::SharedProgram debugReflProbeShader;
	glow::SharedProgram precalcEnvBrdfLutShader;
	glow::SharedProgram precalcEnvMapShader;
	glow::SharedProgram precalcEnvMapProbeShader;
	glow::SharedProgram lineShader;
	glow::SharedProgram selectedProbeShader;

	glow::SharedVertexArray vaoQuad;
	glow::SharedVertexArray vaoCube;
	glow::SharedVertexArray vaoSphere;
	glow::SharedVertexArray vaoLine;

	glow::SharedTexture2D topRightDebugTexture;
	glow::SharedTexture2D bottomRightDebugTexture;
	glow::SharedTextureCubeMap debugEnvMap;
	glow::SharedTextureCubeMap skybox;
	glow::SharedTexture2D envLutGGX;
	glow::SharedTextureCubeMap defaultEnvMapGGX;
	glow::SharedTextureCubeMapArray reflectionProbeArray;
	glow::SharedTexture3D probeVisibilityTexture;
	glow::SharedTexture1DArray probeInfluenceTexture;

	const glow::camera::GenericCamera* camera;
	glm::vec3 ambientColor = glm::vec3(0.0f);
	const DirectionalLight* light;
	std::vector<Mesh> texturedMeshes;
	std::vector<Mesh> untexturedMeshes;
	bool useIrradianceMap = true;
	bool useAOMap = true;
	bool useIbl = true;
	bool useLocalProbes = true;
	float bloomPercentage = 0.02f;
	float exposureAdjustment = 1.0f;
	glm::vec3 debugEnvMapPosition;
	int debugEnvMapMipLevel = 0;
	bool showDebugProbeVisGrid = false;
    
    const std::vector<ReflectionProbe>* reflectionProbes;
    bool isDebugProbeGridEnabled = false;
    std::vector<glow::SharedTextureCubeMap> probeGrid;
    std::vector<glm::vec3> probeGridPositions;
    
	glm::vec3 probeVisibilityMin;
	glm::vec3 probeVisibilityMax;
	glm::vec3 probeVisibilityVoxelSize;
	glm::ivec3 probeVisibilityGridDimensions;

	int currentProbeIndex = -1;
	bool probePlacementPreviewEnabled = false;
	glm::vec3 probePlacementPreviewPos = glm::vec3(0);
};
