#include "limits.hh"

#include "glow.hh"

#include "gl.hh"

int glow::limits::maxCombinedTextureImageUnits = -1;
int glow::limits::maxPatchVertices = -1;
float glow::limits::maxAnisotropy = -1;

void glow::limits::update()
{
    checkValidGLOW();

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);
    glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxPatchVertices);
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
}
