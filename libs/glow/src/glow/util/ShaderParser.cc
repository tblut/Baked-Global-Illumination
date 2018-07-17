#include "ShaderParser.hh"

#include <glow/objects/Shader.hh>

using namespace glow;

ShaderParser::ShaderParser()
{
}

void ShaderParser::addDependency(Shader *shader, const std::string &filename) const
{
    shader->addDependency(filename);
}

ShaderParser::~ShaderParser()
{
}
