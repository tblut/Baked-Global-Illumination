#pragma once

#include "Mesh.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/camera/CameraBase.hh>
#include <vector>

class RenderPipeline {
public:
	RenderPipeline();

	void render(const glow::camera::CameraBase& camera, const std::vector<Mesh>& meshes);
	void resizeBuffers(int w, int h);

	void setAmbientColor(const glm::vec3& color);
	void setLightDirection(const glm::vec3& direction);
	void setLightColor(const glm::vec3& color);

private:
	glow::SharedTextureRectangle hdrColorBuffer;
	glow::SharedTextureRectangle brightnessBuffer;
	glow::SharedTextureRectangle depthBuffer;
	glow::SharedFramebuffer hdrFbo;

	glow::SharedTextureRectangle blurColorBufferA;
	glow::SharedTextureRectangle blurColorBufferB;
	glow::SharedFramebuffer blurFboA;
	glow::SharedFramebuffer blurFboB;

	glow::SharedProgram objectShader;
	glow::SharedProgram objectNoTexShader;
	glow::SharedProgram skyboxShader;
	glow::SharedProgram downsampleShader;
	glow::SharedProgram blurShader;
	glow::SharedProgram postProcessShader;

	glow::SharedVertexArray vaoQuad;
	glow::SharedVertexArray vaoCube;
	glow::SharedVertexArray vertexArray;

	glow::SharedTexture2D textureColor;
	glow::SharedTexture2D textureNormal;
	glow::SharedTextureCubeMap skybox;

	glm::vec3 ambientColor = glm::vec3(0.01f);
	glm::vec3 lightDir = glm::vec3(1, -5, -2);
	glm::vec3 lightColor = glm::vec3(1.0f, 0.9f, 0.8f);
};