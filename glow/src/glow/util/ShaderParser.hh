#pragma once

#include <string>
#include <vector>

#include <glow/gl.hh>

namespace glow
{
class Shader;

/**
 * @brief The ShaderParser class is used for parsing shader source and resolving extra functionality like #include
 *
 */
class ShaderParser
{
protected:
    ShaderParser();

    /// Registers a file as dependency for a shader
    void addDependency(Shader* shader, std::string const& filename) const;

public:
    /// Parses the source of a shader
    /// Adds and resolves dependencies on demand
    /// shader->getFileName() is valid at this point
    virtual std::vector<std::string> parse(Shader* shader, std::vector<std::string> const& sources) = 0;

    /// Returns true if the provided name can be resolved to a shader
    /// (returns type and content seperately)
    /// Simple implementations can just check if the file exists on the drive
    /// Complex versions can check include pathes or virtual filesystems
    /// realFileName is non-empty if the shader is file-backed
    virtual bool resolveFile(std::string const& name, GLenum& shaderType, std::string& content, std::string& realFileName) = 0;

public:
    virtual ~ShaderParser();
};
}
