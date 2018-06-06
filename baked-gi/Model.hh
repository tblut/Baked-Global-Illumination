#pragma once

#include <glow/fwd.hh>
#include <glow/objects/Program.hh>

#include <vector>
#include <string>
#include <unordered_map>

class Model {
public:
	void loadFromFile(const std::string& path, const std::string& texturesPath);
	void render(glow::Program::UsedProgram& shader, bool texturedPass) const;

private:
	struct Mesh {
		glow::SharedVertexArray vao;
		glow::SharedTexture2D colorMap;
		glow::SharedTexture2D normalMap;
		float roughness = 0.5f;
		float metallic = 0.0f;
		glm::vec3 baseColor = glm::vec3(1.0);
	};

	std::unordered_map<std::string, glow::SharedTexture2D> textures;
	std::vector<Mesh> meshes;
	glow::SharedTexture2D defaultNormalMap;
};