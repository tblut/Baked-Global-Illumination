#include "shader_endings.hh"

const std::map<std::string, GLenum> glow::shaderEndingToType = {
    {".vsh", GL_VERTEX_SHADER},           //
    {".fsh", GL_FRAGMENT_SHADER},         //
    {".gsh", GL_GEOMETRY_SHADER},         //
    {".csh", GL_COMPUTE_SHADER},          //
    {".tcsh", GL_TESS_CONTROL_SHADER},    //
    {".tesh", GL_TESS_EVALUATION_SHADER}, //
};
