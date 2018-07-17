#include "Program.hh"

#include "AtomicCounterBuffer.hh"
#include "ShaderStorageBuffer.hh"
#include "Texture.hh"
#include "Texture2D.hh"
#include "UniformBuffer.hh"

#include "glow/glow.hh"
#include "glow/limits.hh"
#include "glow/util/UniformState.hh"

#include "glow/common/runtime_assert.hh"

using namespace glow;

void Program::UsedProgram::setUniform(const std::string &name, int count, const GLfloat *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glUniform1fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT), count, values);
}

void Program::UsedProgram::setTexture(const std::string &name, const SharedTexture &tex)
{
    if (!isCurrent())
        return;

    checkValidGLOW();

    // get unit
    auto unit = program->mTextureUnitMapping.getOrAddLocation(name);

    if (tex)
    {
        // bind texture to unit
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(tex->getTarget(), tex->getObjectName());

        // safety net: activate different unit
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);

        // update shader binding
        glUniform1i(program->useUniformLocationAndVerify(name, 1, tex->getUniformType()), unit);

        // save texture
        while (program->mTextures.size() <= unit)
            program->mTextures.push_back(nullptr);
        program->mTextures[unit] = tex;
    }
    else // nullptr
    {
        // erase texture entry
        if (unit < program->mTextures.size())
            program->mTextures[unit] = nullptr;

        // update shader binding
        glUniform1i(program->useUniformLocationAndVerify(name, 1, tex->getUniformType()), limits::maxCombinedTextureImageUnits - 1);
    }
}

void Program::UsedProgram::setImage(int bindingLocation, const SharedTexture &tex, GLenum usage, int mipmapLevel, int layer)
{
    GLOW_RUNTIME_ASSERT(tex->isStorageImmutable(), "Texture has to be storage immutable for image binding", return );
    checkValidGLOW();

    // TODO: format handling
    // TODO: slicing
    glBindImageTexture(bindingLocation, tex->getObjectName(), mipmapLevel, GL_TRUE, layer, usage, tex->getInternalFormat());
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const bool *values) const
{
    std::vector<int32_t> tmp(count);
    for (auto i = 0; i < count; ++i)
        tmp[i] = (int)values[i];
    setUniformBool(name, tmp.size(), tmp.data());
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const int32_t *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glUniform1iv(program->useUniformLocationAndVerify(name, count, GL_INT), count, values);
}

void Program::UsedProgram::setUniformIntInternal(const std::string &name, int count, const int32_t *values, GLenum uniformType) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glUniform1iv(program->useUniformLocationAndVerify(name, count, uniformType), count, values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const uint32_t *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    glUniform1uiv(program->useUniformLocationAndVerify(name, count, GL_UNSIGNED_INT), count, values);
}

void Program::UsedProgram::setUniformBool(const std::string &name, int count, const int32_t *values) const
{
    checkValidGLOW();
    glUniform1iv(program->useUniformLocationAndVerify(name, count, GL_BOOL), count, values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::vec2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::vec2) == 2 * sizeof(GLfloat), "glm size check");
    glUniform2fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_VEC2), count, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::vec3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::vec3) == 3 * sizeof(GLfloat), "glm size check");
    glUniform3fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_VEC3), count, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::vec4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::vec4) == 4 * sizeof(GLfloat), "glm size check");
    glUniform4fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_VEC4), count, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::ivec2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::ivec2) == 2 * sizeof(GLint), "glm size check");
    glUniform2iv(program->useUniformLocationAndVerify(name, count, GL_INT_VEC2), count, (GLint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::ivec3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::ivec3) == 3 * sizeof(GLint), "glm size check");
    glUniform3iv(program->useUniformLocationAndVerify(name, count, GL_INT_VEC3), count, (GLint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::ivec4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::ivec4) == 4 * sizeof(GLint), "glm size check");
    glUniform4iv(program->useUniformLocationAndVerify(name, count, GL_INT_VEC4), count, (GLint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::uvec2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::uvec2) == 2 * sizeof(GLuint), "glm size check");
    glUniform2uiv(program->useUniformLocationAndVerify(name, count, GL_UNSIGNED_INT_VEC2), count, (GLuint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::uvec3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::uvec3) == 3 * sizeof(GLuint), "glm size check");
    glUniform3uiv(program->useUniformLocationAndVerify(name, count, GL_UNSIGNED_INT_VEC3), count, (GLuint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::uvec4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::uvec4) == 4 * sizeof(GLuint), "glm size check");
    glUniform4uiv(program->useUniformLocationAndVerify(name, count, GL_UNSIGNED_INT_VEC4), count, (GLuint *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::bvec2 *values) const
{
    if (!isCurrent())
        return;

    std::vector<glm::ivec2> tmp(count);
    for (auto i = 0; i < count; ++i)
        tmp[i] = (glm::ivec2)values[i];

    checkValidGLOW();
    static_assert(sizeof(glm::ivec2) == 2 * sizeof(GLint), "glm size check");
    glUniform2iv(program->useUniformLocationAndVerify(name, count, GL_BOOL_VEC2), count, (GLint *)tmp.data());
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::bvec3 *values) const
{
    if (!isCurrent())
        return;

    std::vector<glm::ivec3> tmp(count);
    for (auto i = 0; i < count; ++i)
        tmp[i] = (glm::ivec3)values[i];

    checkValidGLOW();
    static_assert(sizeof(glm::ivec3) == 3 * sizeof(GLint), "glm size check");
    glUniform3iv(program->useUniformLocationAndVerify(name, count, GL_BOOL_VEC3), count, (GLint *)tmp.data());
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::bvec4 *values) const
{
    if (!isCurrent())
        return;

    std::vector<glm::ivec4> tmp(count);
    for (auto i = 0; i < count; ++i)
        tmp[i] = (glm::ivec4)values[i];

    checkValidGLOW();
    static_assert(sizeof(glm::ivec4) == 4 * sizeof(GLint), "glm size check");
    glUniform4iv(program->useUniformLocationAndVerify(name, count, GL_BOOL_VEC4), count, (GLint *)tmp.data());
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat2x2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat2x2) == 2 * 2 * sizeof(GLfloat), "glm size check");
    glUniformMatrix2fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT2), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat3x3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat3x3) == 3 * 3 * sizeof(GLfloat), "glm size check");
    glUniformMatrix3fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT3), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat4x4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat4x4) == 4 * 4 * sizeof(GLfloat), "glm size check");
    glUniformMatrix4fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT4), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat2x3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat2x3) == 2 * 3 * sizeof(GLfloat), "glm size check");
    glUniformMatrix2x3fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT2x3), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat2x4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat2x4) == 2 * 4 * sizeof(GLfloat), "glm size check");
    glUniformMatrix2x4fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT2x4), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat3x2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat3x2) == 3 * 2 * sizeof(GLfloat), "glm size check");
    glUniformMatrix3x2fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT3x2), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat3x4 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat3x4) == 3 * 4 * sizeof(GLfloat), "glm size check");
    glUniformMatrix3x4fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT3x4), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat4x2 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat4x2) == 4 * 2 * sizeof(GLfloat), "glm size check");
    glUniformMatrix4x2fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT4x2), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniform(const std::string &name, int count, const glm::mat4x3 *values) const
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    static_assert(sizeof(glm::mat4x3) == 4 * 3 * sizeof(GLfloat), "glm size check");
    glUniformMatrix4x3fv(program->useUniformLocationAndVerify(name, count, GL_FLOAT_MAT4x3), count, GL_FALSE, (GLfloat *)values);
}

void Program::UsedProgram::setUniforms(const SharedUniformState &state)
{
    if (!isCurrent())
        return;

    state->restore();
}

void Program::UsedProgram::setUniform(const std::string &name, GLenum uniformType, GLint size, void const *data)
{
    switch (uniformType)
    {
    // floats
    case GL_FLOAT:
        setUniform(name, size, (float const *)data);
        break;
    case GL_FLOAT_VEC2:
        setUniform(name, size, (glm::vec2 const *)data);
        break;
    case GL_FLOAT_VEC3:
        setUniform(name, size, (glm::vec3 const *)data);
        break;
    case GL_FLOAT_VEC4:
        setUniform(name, size, (glm::vec4 const *)data);
        break;
    case GL_FLOAT_MAT2:
        setUniform(name, size, (glm::mat2 const *)data);
        break;
    case GL_FLOAT_MAT3:
        setUniform(name, size, (glm::mat3 const *)data);
        break;
    case GL_FLOAT_MAT4:
        setUniform(name, size, (glm::mat4 const *)data);
        break;
    case GL_FLOAT_MAT2x3:
        setUniform(name, size, (glm::mat2x3 const *)data);
        break;
    case GL_FLOAT_MAT2x4:
        setUniform(name, size, (glm::mat2x4 const *)data);
        break;
    case GL_FLOAT_MAT3x2:
        setUniform(name, size, (glm::mat3x2 const *)data);
        break;
    case GL_FLOAT_MAT3x4:
        setUniform(name, size, (glm::mat3x4 const *)data);
        break;
    case GL_FLOAT_MAT4x2:
        setUniform(name, size, (glm::mat4x2 const *)data);
        break;
    case GL_FLOAT_MAT4x3:
        setUniform(name, size, (glm::mat4x3 const *)data);
        break;

    // doubles
    /* MAYBE some day
case GL_DOUBLE:
    setUniform(name, size, (double const*)data);
    break;
case GL_DOUBLE_VEC2:
    setUniform(name, size, (glm::dvec2 const*)data);
    break;
case GL_DOUBLE_VEC3:
    setUniform(name, size, (glm::dvec3 const*)data);
    break;
case GL_DOUBLE_VEC4:
    setUniform(name, size, (glm::dvec4 const*)data);
    break;
case GL_DOUBLE_MAT2:
    setUniform(name, size, (glm::dmat2 const*)data);
    break;
case GL_DOUBLE_MAT3:
    setUniform(name, size, (glm::dmat3 const*)data);
    break;
case GL_DOUBLE_MAT4:
    setUniform(name, size, (glm::dmat4 const*)data);
    break;
case GL_DOUBLE_MAT2x3:
    setUniform(name, size, (glm::dmat2x3 const*)data);
    break;
case GL_DOUBLE_MAT2x4:
    setUniform(name, size, (glm::dmat2x4 const*)data);
    break;
case GL_DOUBLE_MAT3x2:
    setUniform(name, size, (glm::dmat3x2 const*)data);
    break;
case GL_DOUBLE_MAT3x4:
    setUniform(name, size, (glm::dmat3x4 const*)data);
    break;
case GL_DOUBLE_MAT4x2:
    setUniform(name, size, (glm::dmat4x2 const*)data);
    break;
case GL_DOUBLE_MAT4x3:
    setUniform(name, size, (glm::dmat4x3 const*)data);
    break;
    */

    // uint
    case GL_UNSIGNED_INT:
    case GL_UNSIGNED_INT_ATOMIC_COUNTER:
        setUniform(name, size, (uint32_t const *)data);
        break;
    case GL_UNSIGNED_INT_VEC2:
        setUniform(name, size, (glm::uvec2 const *)data);
        break;
    case GL_UNSIGNED_INT_VEC3:
        setUniform(name, size, (glm::uvec3 const *)data);
        break;
    case GL_UNSIGNED_INT_VEC4:
        setUniform(name, size, (glm::uvec4 const *)data);
        break;

    // int
    case GL_INT:
        setUniform(name, size, (int32_t const *)data);
        break;
    case GL_INT_VEC2:
        setUniform(name, size, (glm::ivec2 const *)data);
        break;
    case GL_INT_VEC3:
        setUniform(name, size, (glm::ivec3 const *)data);
        break;
    case GL_INT_VEC4:
        setUniform(name, size, (glm::ivec4 const *)data);
        break;
    // bool
    case GL_BOOL:
        setUniformIntInternal(name, size, (int32_t const *)data, GL_BOOL);
        break;
    case GL_BOOL_VEC2:
        setUniform(name, size, (glm::ivec2 const *)data);
        break;
    case GL_BOOL_VEC3:
        setUniform(name, size, (glm::ivec3 const *)data);
        break;
    case GL_BOOL_VEC4:
        setUniform(name, size, (glm::ivec4 const *)data);
        break;
    // sampler
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_1D_ARRAY:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_1D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_2D_MULTISAMPLE:
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_SAMPLER_BUFFER:
    case GL_SAMPLER_2D_RECT:
    case GL_SAMPLER_2D_RECT_SHADOW:
    case GL_INT_SAMPLER_1D:
    case GL_INT_SAMPLER_2D:
    case GL_INT_SAMPLER_3D:
    case GL_INT_SAMPLER_CUBE:
    case GL_INT_SAMPLER_1D_ARRAY:
    case GL_INT_SAMPLER_2D_ARRAY:
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_INT_SAMPLER_BUFFER:
    case GL_INT_SAMPLER_2D_RECT:
    case GL_UNSIGNED_INT_SAMPLER_1D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_3D:
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_SAMPLER_BUFFER:
    case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
    // images
    case GL_IMAGE_1D:
    case GL_IMAGE_2D:
    case GL_IMAGE_3D:
    case GL_IMAGE_CUBE:
    case GL_IMAGE_1D_ARRAY:
    case GL_IMAGE_2D_ARRAY:
    case GL_IMAGE_2D_MULTISAMPLE:
    case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_IMAGE_BUFFER:
    case GL_IMAGE_2D_RECT:
    case GL_INT_IMAGE_1D:
    case GL_INT_IMAGE_2D:
    case GL_INT_IMAGE_3D:
    case GL_INT_IMAGE_CUBE:
    case GL_INT_IMAGE_1D_ARRAY:
    case GL_INT_IMAGE_2D_ARRAY:
    case GL_INT_IMAGE_2D_MULTISAMPLE:
    case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_INT_IMAGE_BUFFER:
    case GL_INT_IMAGE_2D_RECT:
    case GL_UNSIGNED_INT_IMAGE_1D:
    case GL_UNSIGNED_INT_IMAGE_2D:
    case GL_UNSIGNED_INT_IMAGE_3D:
    case GL_UNSIGNED_INT_IMAGE_CUBE:
    case GL_UNSIGNED_INT_IMAGE_1D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE:
    case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY:
    case GL_UNSIGNED_INT_IMAGE_BUFFER:
    case GL_UNSIGNED_INT_IMAGE_2D_RECT:
        setUniformIntInternal(name, size, (int32_t const *)data, uniformType);
        break;

    default:
        error() << "Uniform type not implemented: " << uniformType;
        break;
    }
}

void Program::setUniformBuffer(const std::string &bufferName, const SharedUniformBuffer &buffer)
{
    checkValidGLOW();
    auto loc = mUniformBufferMapping.getOrAddLocation(bufferName);
    auto idx = glGetUniformBlockIndex(mObjectName, bufferName.c_str());

    auto isNew = !mUniformBuffers.count(bufferName);
    mUniformBuffers[bufferName] = buffer;

    if (idx == GL_INVALID_INDEX)
        return; // not active

    glUniformBlockBinding(mObjectName, idx, loc);

    if (getCurrentProgram() && getCurrentProgram()->program == this)
        glBindBufferBase(GL_UNIFORM_BUFFER, loc, buffer ? buffer->getObjectName() : 0);

    if (isNew)
        verifyUniformBuffer(bufferName, buffer);
}

void Program::setShaderStorageBuffer(const std::string &bufferName, const SharedShaderStorageBuffer &buffer)
{
    checkValidGLOW();
    auto loc = mShaderStorageBufferMapping.getOrAddLocation(bufferName);
    auto idx = glGetProgramResourceIndex(mObjectName, GL_SHADER_STORAGE_BLOCK, bufferName.c_str());

    mShaderStorageBuffers[bufferName] = buffer;

    if (idx == GL_INVALID_INDEX)
        return; // not active

    glShaderStorageBlockBinding(mObjectName, idx, loc);

    if (getCurrentProgram() && getCurrentProgram()->program == this)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, loc, buffer ? buffer->getObjectName() : 0);
}

void Program::setAtomicCounterBuffer(int bindingPoint, const SharedAtomicCounterBuffer &buffer)
{
    checkValidGLOW();
    mAtomicCounterBuffers[bindingPoint] = buffer;

    if (getCurrentProgram() && getCurrentProgram()->program == this)
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, bindingPoint, buffer ? buffer->getObjectName() : 0);
}

bool Program::verifyUniformBuffer(const std::string &bufferName, const SharedUniformBuffer &buffer)
{
    if (buffer->getVerificationOffsets().empty())
        return true; // nothing to verify

    checkValidGLOW();

    auto blockIdx = glGetUniformBlockIndex(mObjectName, bufferName.c_str());
    if (blockIdx == GL_INVALID_INDEX)
        return true; // not active

    // get nr of uniforms
    GLint uniformCnt = -1;
    glGetActiveUniformBlockiv(mObjectName, blockIdx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCnt);

    // get uniform indices
    std::vector<GLint> uniformIndices;
    uniformIndices.resize(uniformCnt);
    glGetActiveUniformBlockiv(mObjectName, blockIdx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices.data());

    // get offsets of uniforms
    std::vector<GLint> uniformOffsets;
    uniformOffsets.resize(uniformCnt);
    glGetActiveUniformsiv(mObjectName, uniformCnt, (const GLuint *)uniformIndices.data(), GL_UNIFORM_OFFSET,
                          uniformOffsets.data());

    // get name max length
    GLint nameMaxLength;
    glGetProgramiv(mObjectName, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameMaxLength);

    // get names
    std::vector<char> nameBuffer;
    nameBuffer.resize(nameMaxLength + 1);
    std::vector<std::string> uniformNames;
    for (auto uIdx : uniformIndices)
    {
        glGetActiveUniformName(mObjectName, uIdx, nameBuffer.size(), nullptr, nameBuffer.data());
        uniformNames.push_back(nameBuffer.data());
    }

    // actual verification
    auto failure = false;
    auto const &vOffsets = buffer->getVerificationOffsets();
    for (auto i = 0u; i < uniformIndices.size(); ++i)
    {
        auto gpuOffset = uniformOffsets[i];
        auto const &name = uniformNames[i];

        if (vOffsets.count(name))
        {
            auto cpuOffset = (int)vOffsets.at(name);

            // mismatch
            if (cpuOffset != gpuOffset)
            {
                if (!failure)
                    error() << "UniformBuffer Verification Failure for `" << bufferName << "':";
                failure = true;

                error() << "  * Uniform `" << name << "': CPU@" << cpuOffset << " vs GPU@" << gpuOffset;
            }
        }
    }

    return !failure;
}
