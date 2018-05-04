#include "Model.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/glfw/GlfwApp.hh>

class BakedGIApp : public glow::glfw::GlfwApp {
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

	glm::vec3 baseColor = glm::vec3(0.5f);
	float roughness = 0.2f;
	float metallic = 0.8f;
	glm::vec3 ambientColor = glm::vec3(0.01f);
	glm::vec3 lightDir = glm::vec3(1, -5, -2);
	glm::vec3 lightColor = glm::vec3(1.5f);

	Model model;

protected:
	virtual void init() override;
	virtual void render(float elapsedSeconds) override;
	virtual void onResize(int w, int h) override;
};