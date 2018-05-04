#pragma once

#include <glow/gl.hh>
#include <glow/glow.hh>

#include <sstream>
#include <string>
#include <vector>

namespace glow
{
template <typename T, GLenum glNamespace>
class NamedObject
{
public:
    // returns the OpenGL Object Label
    std::string getObjectLabel() const
    {
        checkValidGLOW();

        GLsizei len = 0;
        glGetObjectLabel(glNamespace, static_cast<T const *>(this)->getObjectName(), 0, &len, nullptr);

        std::vector<char> label;
        label.resize(len + 1);
        glGetObjectLabel(glNamespace, static_cast<T const *>(this)->getObjectName(), len + 1, nullptr, label.data());

        return label.data(); // convert to string
    }

    // sets the OpenGL Object Label
    void setObjectLabel(const std::string &label)
    {
        checkValidGLOW();

        glObjectLabel(glNamespace, static_cast<T const *>(this)->getObjectName(), -1, label.c_str());
    }
};

template <typename T, GLenum glNamespace>
std::string to_string(NamedObject<T, glNamespace> const *obj)
{
    std::ostringstream oss;
    oss << "[";
    switch (glNamespace)
    {
    case GL_BUFFER:
        oss << "Buffer ";
        break;
    case GL_SHADER:
        oss << "Shader ";
        break;
    case GL_PROGRAM:
        oss << "Program ";
        break;
    case GL_VERTEX_ARRAY:
        oss << "VertexArray ";
        break;
    case GL_QUERY:
        oss << "Query ";
        break;
    case GL_PROGRAM_PIPELINE:
        oss << "ProgramPipeline ";
        break;
    case GL_TRANSFORM_FEEDBACK:
        oss << "TransformFeedback ";
        break;
    case GL_SAMPLER:
        oss << "Sampler ";
        break;
    case GL_TEXTURE:
        oss << "Texture ";
        break;
    case GL_RENDERBUFFER:
        oss << "Renderbuffer ";
        break;
    case GL_FRAMEBUFFER:
        oss << "Framebuffer ";
        break;
    default:
        oss << "UNKNOWN ";
        break;
    }
    oss << static_cast<T const *>(obj)->getObjectName();
    auto label = obj->getObjectLabel();
    if (!label.empty())
        oss << ": " << label;
    oss << "]";
    return oss.str();
}
}
