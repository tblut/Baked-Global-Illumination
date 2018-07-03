#include "CubeMap.hh"
#include "third-party/stb_image.h"
#include <glow/common/log.hh>
#include <cstring>

glm::vec3 CubeMap::sample(const glm::vec3& dir) const {
    glm::vec3 absDir = glm::abs(dir);
    if (absDir.x > absDir.y && absDir.x > absDir.z) {
        if (dir.x > 0) {
            glm::vec2 uv = (glm::vec2(-dir.z / absDir.x, dir.y / absDir.x) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[0]->sample(uv));
        }
        else {
            glm::vec2 uv = (glm::vec2(dir.z / absDir.x, dir.y / absDir.x) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[1]->sample(uv));
        }
    }
    else if (absDir.y > absDir.x && absDir.y > absDir.z) {
        if (dir.y > 0) {
            glm::vec2 uv = (glm::vec2(dir.x / absDir.y, -dir.z / absDir.y) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[2]->sample(uv));
        }
        else {
            glm::vec2 uv = (glm::vec2(dir.x / absDir.y, dir.z / absDir.y) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[3]->sample(uv));
        }
    }
    else if (absDir.z > absDir.x && absDir.z > absDir.y) {
        if (dir.z > 0) {
            glm::vec2 uv = (glm::vec2(dir.x / absDir.z, dir.y / absDir.z) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[4]->sample(uv));
        }
        else {
            glm::vec2 uv = (glm::vec2(-dir.x / absDir.z, dir.y / absDir.z) + glm::vec2(1.0f)) * 0.5f;
            return glm::vec3(faces[5]->sample(uv));
        }
    }

	return glm::vec3(0.0f);
}

SharedCubeMap CubeMap::loadFromFiles(const std::string& posX, const std::string& negX,
                              const std::string& posY, const std::string& negY,
                              const std::string& posZ, const std::string& negZ) {
    auto cubeMap = std::make_shared<CubeMap>();
    std::string filenames[] = { posX, negX, posY, negY, posZ, negZ };
    for (int i = 0; i < 6; ++i) {
        int width, height, channel;
        unsigned char* data = stbi_load(filenames[i].c_str(), &width, &height, &channel, 0);
		cubeMap->faces[i] = std::make_shared<Image>(width, height, GL_SRGB);
        std::memcpy(cubeMap->faces[i]->getDataPtr(), data, width * height * channel);
        stbi_image_free(data);
    }
    return cubeMap;
}
