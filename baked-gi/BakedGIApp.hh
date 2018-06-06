#pragma once

#include "Model.hh"
#include "Scene.hh"
#include "RenderPipeline.hh"

#include <glm/ext.hpp>
#include <glow/fwd.hh>
#include <glow-extras/glfw/GlfwApp.hh>

#include <memory>

class BakedGIApp : public glow::glfw::GlfwApp {
protected:
	virtual void init() override;
	virtual void render(float elapsedSeconds) override;
	virtual void onResize(int w, int h) override;

private:
	//Model model;
	Scene scene;
	std::unique_ptr<RenderPipeline> pipeline;

	glm::vec3 ambientColor = glm::vec3(0.01f);
	glm::vec3 lightDir = glm::vec3(1, -5, -2);
	glm::vec3 lightColor = glm::vec3(1.0f, 0.9f, 0.8f);
};