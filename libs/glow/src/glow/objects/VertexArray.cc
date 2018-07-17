#include "VertexArray.hh"

#include <algorithm>
#include <cassert>
#include <limits>

#include "ArrayBuffer.hh"
#include "ElementArrayBuffer.hh"
#include "Framebuffer.hh"
#include "Program.hh"
#include "TransformFeedback.hh"

#include "glow/callbacks.hh"
#include "glow/common/log.hh"
#include "glow/common/runtime_assert.hh"
#include "glow/common/thread_local.hh"
#include "glow/glow.hh"
#include "glow/util/LocationMapping.hh"
#include "glow/util/UniformState.hh"

using namespace glow;

/// Currently bound VAO
static GLOW_THREADLOCAL VertexArray::BoundVertexArray *sCurrentVAO = nullptr;

VertexArray::BoundVertexArray *VertexArray::getCurrentVAO()
{
    return sCurrentVAO;
}

VertexArray::VertexArray(GLenum primitiveMode) : mPrimitiveMode(primitiveMode)
{
    checkValidGLOW();
    mObjectName = std::numeric_limits<decltype(mObjectName)>::max();
    glGenVertexArrays(1, &mObjectName);
    assert(mObjectName != std::numeric_limits<decltype(mObjectName)>::max() && "No OpenGL Context?");

    // bind the VA once to guarantee that object is valid
    GLint prevVA = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevVA);
    glBindVertexArray(mObjectName);
    // VA is now valid
    glBindVertexArray(prevVA);

    mAttributeMapping = std::make_shared<LocationMapping>();
}

VertexArray::~VertexArray()
{
    checkValidGLOW();
    glDeleteVertexArrays(1, &mObjectName);
}

SharedVertexArray VertexArray::create(GLenum primitiveMode)
{
    return std::make_shared<VertexArray>(primitiveMode);
}

SharedVertexArray VertexArray::create(const SharedArrayBuffer &ab, const SharedElementArrayBuffer &eab, GLenum primitiveMode)
{
    auto vao = create(primitiveMode);
    auto bvao = vao->bind();
    bvao.attach(ab);
    bvao.attach(eab);
    return vao;
}

SharedVertexArray VertexArray::create(const SharedArrayBuffer &ab, GLenum primitiveMode)
{
    auto vao = create(primitiveMode);
    auto bvao = vao->bind();
    bvao.attach(ab);
    return vao;
}

SharedVertexArray VertexArray::create(const std::vector<SharedArrayBuffer> &abs, const SharedElementArrayBuffer &eab, GLenum primitiveMode)
{
    auto vao = create(primitiveMode);
    auto bvao = vao->bind();
    bvao.attach(abs);
    bvao.attach(eab);
    return vao;
}

void VertexArray::BoundVertexArray::updatePatchParameters()
{
    if (vao->mPrimitiveMode == GL_PATCHES)
    {
        if (vao->mVerticesPerPatch <= 1)
            glow::error() << "Invalid number of patches per vertices (forgot to call setVerticesPerPatch?) " << to_string(vao);
        glPatchParameteri(GL_PATCH_VERTICES, vao->mVerticesPerPatch);
    }
}

void VertexArray::BoundVertexArray::draw(GLsizei instanceCount)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    negotiateBindings();
    instanceCount = instanceCount >= 0 ? instanceCount : vao->getInstanceCount();

    if (vao->isEmpty())
        glow::warning() << "Drawing empty VertexArray (did you forget to call ArrayBuffer::setData(...) or "
                           "VertexArray::attach(...)?). "
                        << to_string(vao);

    if (Program::getCurrentProgram() == nullptr)
        glow::warning() << "Drawing without any shader used (did you forget to call Program::use()?). " << to_string(vao);

    updatePatchParameters();

    if (vao->mElementArrayBuffer)
    {
        // draw indexed
        glDrawElementsInstanced(vao->mPrimitiveMode, vao->mElementArrayBuffer->getIndexCount(),
                                vao->mElementArrayBuffer->getIndexType(), nullptr, instanceCount);
    }
    else
    {
        // draw unindexed
        glDrawArraysInstanced(vao->mPrimitiveMode, 0, vao->getVertexCount(), instanceCount);
    }

    // notify FBOs
    notifyShaderExecuted();
}

void VertexArray::BoundVertexArray::drawRange(GLsizei start, GLsizei end, GLsizei instanceCount)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    negotiateBindings();
    instanceCount = instanceCount >= 0 ? instanceCount : vao->getInstanceCount();

    if (vao->isEmpty())
        glow::warning() << "Drawing empty VertexArray (did you forget to call ArrayBuffer::setData(...) or "
                           "VertexArray::attach(...)?). "
                        << to_string(vao);

    if (Program::getCurrentProgram() == nullptr)
        glow::warning() << "Drawing without any shader used (did you forget to call Program::use()?). " << to_string(vao);

    updatePatchParameters();

    if (vao->mElementArrayBuffer)
    {
        // draw indexed
        auto indexSize = ptrdiff_t{0};
        switch (vao->mElementArrayBuffer->getIndexType())
        {
        case GL_UNSIGNED_BYTE:
            indexSize = 1;
            break;
        case GL_UNSIGNED_SHORT:
            indexSize = 2;
            break;
        case GL_UNSIGNED_INT:
            indexSize = 4;
            break;

        default:
            error() << "Unknown index type: " << vao->mElementArrayBuffer->getIndexType() << " " << to_string(vao);
            return;
        }

        glDrawElementsInstanced(vao->mPrimitiveMode, end - start, vao->mElementArrayBuffer->getIndexType(),
                                (void *)(start * indexSize), instanceCount);
    }
    else
    {
        // draw unindexed
        glDrawArraysInstanced(vao->mPrimitiveMode, start, end - start, instanceCount);
    }

    // notify FBOs
    notifyShaderExecuted();
}

void VertexArray::BoundVertexArray::drawTransformFeedback(const SharedTransformFeedback &feedback)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    negotiateBindings();

    if (Program::getCurrentProgram() == nullptr)
        glow::warning() << "Drawing without any shader used (did you forget to call Program::use()?). " << to_string(vao);

    updatePatchParameters();

    GLOW_RUNTIME_ASSERT(vao->mElementArrayBuffer == nullptr,
                        "Cannot use drawFeedback with an index buffer " << to_string(vao), return );
    GLOW_RUNTIME_ASSERT(!feedback->isBound(), "Cannot use drawFeedback when feedback is still bound " << to_string(vao), return );

    glDrawTransformFeedback(vao->mPrimitiveMode, feedback->getObjectName());

    // notify FBOs
    notifyShaderExecuted();
}

void VertexArray::BoundVertexArray::attach(const SharedElementArrayBuffer &eab)
{
    if (!isCurrent())
        return;

    checkValidGLOW();
    vao->mElementArrayBuffer = eab;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab ? eab->getObjectName() : 0);
}

void VertexArray::attachAttribute(const VertexArrayAttribute &va)
{
    checkValidGLOW();
    auto boundAB = va.buffer->bind(); // important, buffer needs to be bound.

    auto const &a = va.buffer->getAttributes()[va.locationInBuffer];
    auto stride = va.buffer->getStride();

    // declare the data
    switch (a.mode)
    {
    case AttributeMode::Float:
        glVertexAttribPointer(va.locationInVAO, a.size, a.type, GL_FALSE, stride, reinterpret_cast<const GLvoid *>(a.offset));
        break;
    case AttributeMode::NormalizedInteger:
        glVertexAttribPointer(va.locationInVAO, a.size, a.type, GL_TRUE, stride, reinterpret_cast<const GLvoid *>(a.offset));
        break;
    case AttributeMode::Integer:
        glVertexAttribIPointer(va.locationInVAO, a.size, a.type, stride, reinterpret_cast<const GLvoid *>(a.offset));
        break;
    case AttributeMode::Double:
#if GLOW_OPENGL_VERSION >= 41
        glVertexAttribLPointer(va.locationInVAO, a.size, a.type, stride, reinterpret_cast<const GLvoid *>(a.offset));
#else
        warning() << "Real 64bit precision double attributes are only supported in OpenGL 4.1+ " << to_string(this);
#endif
        break;

    default:
        assert(0 && "not implemented");
    }

    // setup divisor
    glVertexAttribDivisor(va.locationInVAO, a.divisor);

    // last step: enable the attribute
    glEnableVertexAttribArray(va.locationInVAO);
}

bool VertexArray::isEmpty() const
{
    // index Cnt > 0 ?
    if (mElementArrayBuffer)
        return mElementArrayBuffer->getIndexCount() == 0;

    // vertex Cnt > 0 ?
    for (auto const &a : mAttributes)
        if (a.buffer->getAttributes()[a.locationInBuffer].divisor == 0)
            return a.buffer->getElementCount() == 0;

    // no divisor 0 attribute
    return true;
}

SharedArrayBuffer VertexArray::getAttributeBuffer(const std::string &name) const
{
    for (auto const &a : mAttributes)
        if (a.buffer->getAttributes()[a.locationInBuffer].name == name)
            return a.buffer;
    return nullptr;
}

int VertexArray::getInstanceCount() const
{
    auto isInstanced = false;
    auto iCnt = std::numeric_limits<int>::max();
    for (auto const &a : mAttributes)
    {
        auto eCnt = a.buffer->getElementCount();
        auto const &aa = a.buffer->getAttributes()[a.locationInBuffer];

        if (aa.divisor > 0)
        {
            isInstanced = true;
            // in order to not sample out of bounds:
            //   at most divisor * |element| instances
            iCnt = std::min(iCnt, (int)(aa.divisor * eCnt));
        }
    }
    return isInstanced ? iCnt : 1;
}

int VertexArray::getVertexCount() const
{
    // vertex count is derived from first divisor-0 attribute
    for (auto const &a : mAttributes)
        if (a.buffer->getAttributes()[a.locationInBuffer].divisor == 0)
            return a.buffer->getElementCount();

    return 0;
}

void VertexArray::BoundVertexArray::attach(const SharedArrayBuffer &ab)
{
    if (!isCurrent())
        return;

    if (!ab)
    {
        error() << "Trying to attach nullptr as ArrayBuffer " << to_string(vao);
        assert(false);
        return;
    }

    auto const &attrs = ab->getAttributes();
    for (auto i = 0u; i < attrs.size(); ++i)
    {
        auto const &a = attrs[i];
        auto loc = vao->mAttributeMapping->getOrAddLocation(a.name);

        VertexArrayAttribute va = {ab, i, loc};
        vao->mAttributes.push_back(va);

        attachAttribute(va);
    }
}

void VertexArray::BoundVertexArray::attach(const std::vector<SharedArrayBuffer> &abs)
{
    if (!isCurrent())
        return;

    for (auto const &ab : abs)
        attach(ab);
}

void VertexArray::BoundVertexArray::reattach()
{
    checkValidGLOW();
    int attribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attribs);

    // disable all
    for (auto loc = 0; loc < attribs; ++loc)
        glDisableVertexAttribArray(loc);

    // update mappings
    for (auto &va : vao->mAttributes)
    {
        auto loc = vao->mAttributeMapping->queryLocation(va.buffer->getAttributes()[va.locationInBuffer].name);
        assert(loc >= 0 && "mapping should be present!");
        va.locationInVAO = loc;
    }

    // attach newly mapped
    for (auto const &va : vao->mAttributes)
        attachAttribute(va);
}

void VertexArray::BoundVertexArray::negotiateBindings()
{
    // negotiate mappings on the fly
    auto currProg = Program::getCurrentProgram();
    if (currProg)
    {
        bool changedVAO = false;
        bool changedProgramA = false;
        bool changedProgramB = false;
        bool changedFBO = false;
        LocationMapping::negotiate(vao->mAttributeMapping, currProg->program->mAttributeMapping, changedVAO, changedProgramA);

        auto fbo = Framebuffer::getCurrentBuffer();
        if (fbo)
            LocationMapping::negotiate(fbo->buffer->mFragmentMapping, currProg->program->mFragmentMapping, changedFBO,
                                       changedProgramB, true);

        if (changedProgramA || changedProgramB || changedFBO /* due to lack of frag API */)
        {
            if (changedProgramB || changedFBO)
                currProg->program->resetFragmentLocationCheck(); /* due to lack of frag API */

            // relink (uniforms are restored)
            currProg->program->link();
        }

        if (changedVAO)
        {
            // reattach attributes
            reattach();
        }

        if (changedFBO)
        {
            assert(fbo);
            // reattach fbo
            fbo->reattach();
        }

        // check fbo
        if (changedFBO || changedProgramB)
        {
            assert(fbo);
            fbo->checkComplete();
        }

        // check vao
        if (changedVAO || changedProgramA)
        {
            auto shaderLocs = currProg->program->extractAttributeLocations();
            for (auto const &kvp : shaderLocs)
            {
                auto found = false;
                for (auto const &va : vao->mAttributes)
                    if (va.buffer->getAttributes()[va.locationInBuffer].name == kvp.first)
                    {
                        found = true;
                        break;
                    }

                if (!found)
                {
                    error() << "Shader requires attribute '" << kvp.first << "' but VAO does not define this attribute! "
                            << to_string(vao) << ", " << to_string(currProg->program);
                }
            }
        }
    }
}

VertexArray::BoundVertexArray::BoundVertexArray(VertexArray *vao) : vao(vao)
{
    checkValidGLOW();
    GLOW_RUNTIME_ASSERT(ElementArrayBuffer::getCurrentBuffer() == nullptr,
                        "Cannot bind a VAO while an EAB is bound! (this has unintended side-effects in OpenGL) "
                            << to_string(vao) << ", " << to_string(ElementArrayBuffer::getCurrentBuffer()->buffer),
                        return );

    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &previousEAB);
    glBindVertexArray(vao->mObjectName);

    previousVaoPtr = sCurrentVAO;
    sCurrentVAO = this;
}

bool VertexArray::BoundVertexArray::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentVAO == this, "Currently bound VAO does NOT match represented vao " << to_string(vao), return false);
    return true;
}

VertexArray::BoundVertexArray::BoundVertexArray(VertexArray::BoundVertexArray &&rhs)
  : vao(rhs.vao), previousVAO(rhs.previousVAO), previousEAB(rhs.previousEAB), previousVaoPtr(rhs.previousVaoPtr)
{
    // invalidate rhs
    rhs.previousVAO = -1;
}

VertexArray::BoundVertexArray::~BoundVertexArray()
{
    if (previousVAO != -1) // if valid
    {
        checkValidGLOW();
        glBindVertexArray(previousVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, previousEAB);
        sCurrentVAO = previousVaoPtr;
    }
}
