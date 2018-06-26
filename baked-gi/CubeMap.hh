#pragma once

#include "Image.hh"

#include <glm/glm.hpp>
#include <string>
#include <memory>

using SharedCubeMap = std::shared_ptr<CubeMap>;

class CubeMap {
public:
    glm::vec3 sample(const glm::vec3& dir) const;
    
    static SharedCubeMap loadFromFiles(
        const std::string& posX, const std::string& negX,
        const std::string& posY, const std::string& negY,
        const std::string& posZ, const std::string& negZ);
    
private:
    CubeMap() = default;
    
    SharedImage faces[6]; // +x -x +y -y +z -z
};
