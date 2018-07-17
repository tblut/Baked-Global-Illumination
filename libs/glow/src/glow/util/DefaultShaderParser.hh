#pragma once

#include "ShaderParser.hh"

#include <sstream>
#include <set>

namespace glow
{
/// The default shader parser has the following extra functionality:
/// * resolve #include "filename" (include locally, if not found from include path)
/// * resolve #include <filename> (include from include paths)
/// * includes have an implicit #pragma once
/// * opengl #version is inserted automatically
class DefaultShaderParser : public ShaderParser
{
private:
    /// default include paths (default is "./")
    static std::vector<std::string> sIncludePaths;

    void parseWithInclude(Shader* shader,
                          std::stringstream& parsedSrc,
                          std::string const& source,
                          int sourceIdx,
                          int& nextSrcIdx,
                          std::set<std::string>& includes,
                          std::string const& relativePath);

    /// Attempts to resolve a filename include
    /// Returns "" if not found
    std::string resolve(std::string const& filename, std::string const& relPath);

public:
    DefaultShaderParser();

    /// sets all include paths (without trailing / )
    static void setIncludePaths(std::vector<std::string> const& paths);
    /// adds another include path (without trailing / )
    static void addIncludePath(std::string const& path);

    std::vector<std::string> parse(Shader* shader, std::vector<std::string> const& sources) override;
    bool resolveFile(std::string const& name, GLenum& shaderType, std::string& content, std::string& realFileName) override;
};
}
