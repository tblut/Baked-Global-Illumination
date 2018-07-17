#include "callbacks.hh"

#include "objects/Framebuffer.hh"
#include "objects/Program.hh"


void glow::notifyShaderExecuted()
{
    // notify Shader
    auto shader = Program::getCurrentProgram();
    if (shader)
    {
        shader->program->validateTextureMipmaps();
        shader->program->checkUnchangedUniforms();
    }

    // notify FBO
    auto fbo = Framebuffer::getCurrentBuffer();
    if (fbo != nullptr)
        fbo->buffer->notifyShaderExecuted();
}
