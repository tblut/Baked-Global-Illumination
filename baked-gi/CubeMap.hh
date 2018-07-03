#pragma once

#include "Image.hh"

#include <glm/glm.hpp>
#include <string>
#include <memory>

class CubeMap {
public:
    glm::vec3 sample(const glm::vec3& dir) const;
    
    static std::shared_ptr<CubeMap> loadFromFiles(
        const std::string& posX, const std::string& negX,
        const std::string& posY, const std::string& negY,
        const std::string& posZ, const std::string& negZ);
    
private:
    SharedImage faces[6]; // +x -x +y -y +z -z
};

using SharedCubeMap = std::shared_ptr<CubeMap>;
