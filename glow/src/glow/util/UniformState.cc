#include "UniformState.hh"

#include "glow/glow.hh"

#include "glow/objects/Program.hh"

#include "glow/common/log.hh"
#include "glow/common/ogl_typeinfo.hh"

using namespace glow;

UniformState::UniformState()
{
}

void UniformState::addUniform(const std::string &name, GLenum type, GLint size, const std::vector<char> &data)
{
    mUniforms.push_back({name, type, size, data});
}

void UniformState::restore()
{
    for (auto const &u : mUniforms)
        Program::getCurrentProgram()->setUniform(u.name, u.type, u.size, (void *)u.data.data());
}


SharedUniformState UniformState::create(Program const *prog)
{
    checkValidGLOW();

    auto state = std::make_shared<UniformState>();

    GLint uniformCnt = 0;
    GLint maxNameLength = 0;
    glGetProgramiv(prog->getObjectName(), GL_ACTIVE_UNIFORMS, &uniformCnt);
    glGetProgramiv(prog->getObjectName(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

    if (uniformCnt <= 0 || maxNameLength <= 0)
        return state; // no uniforms

    std::vector<char> name;
    name.resize(maxNameLength);

    for (auto uniformIndex = 0; uniformIndex < uniformCnt; ++uniformIndex)
    {
        GLint uniformSize = 0;
        GLenum uniformType;
        glGetActiveUniform(prog->getObjectName(), uniformIndex, name.size(), nullptr, &uniformSize, &uniformType, name.data());
        if (glGetUniformLocation(prog->getObjectName(), name.data()) == -1)
            continue; // e.g. for uniform buffer
        // warning() << "loc of " << name.data() << ": " << glGetUniformLocation(prog->getObjectName(), name.data());

        std::string uniformName = std::string(name.data());
        GLint loc = prog->getUniformLocation(uniformName);

        std::vector<char> data;

        size_t byteCount = byteSizeOfUniformType(uniformType);

        data.resize(byteCount * uniformSize);

        for (auto arrayIndex = 0; arrayIndex < uniformSize; ++arrayIndex)
        {
            getUniformValue(uniformType, prog->getObjectName(), loc + arrayIndex, data.data() + byteCount * arrayIndex);
        }

        state->mUniforms.push_back({ uniformName, uniformType, uniformSize, data});
    }

    return state;
}
