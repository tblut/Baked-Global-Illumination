#pragma once

#include "Mesh.hh"
#include "DirectionalLight.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/camera/CameraBase.hh>
#include <vector>

class RenderPipeline {
public:
	RenderPipeline();

	void render(const std::vector<Mesh>& meshes);
	void resizeBuffers(int w, int h);

	void setAmbientColor(const glm::vec3& color);
	void attachCamera(const glow::camera::CameraBase& camera);
	void attachLight(const DirectionalLight& light);

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

	const glow::camera::CameraBase* camera;
	glm::vec3 ambientColor = glm::vec3(0.05f);
	const DirectionalLight* light;
};