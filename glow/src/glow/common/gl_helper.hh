#pragma once

#include <glow/gl.hh>

namespace glow
{
/// function-style getter for glGet with one GLenum
inline GLenum glGetEnum(GLenum name)
{
    GLenum val;
    glGetIntegerv(name, (GLint*)&val);
    return val;
}
/// function-style getter for glGet with one GLboolean
inline bool glGetBool(GLenum name)
{
    GLboolean val;
    glGetBooleanv(name, &val);
    return (bool)val;
}
/// function-style getter for glGet with one GLfloat
inline GLfloat glGetFloat(GLenum name)
{
    GLfloat val;
    glGetFloatv(name, &val);
    return val;
}
/// function-style getter for glGet with one GLint
inline GLint glGetInt(GLenum name)
{
    GLint val;
    glGetIntegerv(name, &val);
    return val;
}
}
