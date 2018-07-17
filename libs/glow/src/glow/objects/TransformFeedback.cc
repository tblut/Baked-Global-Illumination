#include "TransformFeedback.hh"

#include "glow/common/log.hh"
#include "glow/glow.hh"

#include <glow/common/runtime_assert.hh>
#include <glow/common/thread_local.hh>

#include "Buffer.hh"
#include "Program.hh"

#include <cassert>
#include <limits>

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL TransformFeedback::BoundTransformFeedback *sCurrentFeedback = nullptr;

TransformFeedback::BoundTransformFeedback *TransformFeedback::getCurrentFeedback()
{
    return sCurrentFeedback;
}

TransformFeedback::TransformFeedback()
{
    checkValidGLOW();

    mObjectName = std::numeric_limits<decltype(mObjectName)>::max();
    glGenTransformFeedbacks(1, &mObjectName);
    assert(mObjectName != std::numeric_limits<decltype(mObjectName)>::max() && "No OpenGL Context?");

    // bind the TF once to guarantee that object is valid
    GLint prevTF = 0;
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_BINDING, &prevTF);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mObjectName);
    // TF is now valid
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, prevTF);
}

TransformFeedback::~TransformFeedback()
{
    checkValidGLOW();

    glDeleteTransformFeedbacks(1, &mObjectName);
}

SharedTransformFeedback TransformFeedback::create(const SharedBuffer &feedbackBuffer)
{
    auto fb = std::make_shared<TransformFeedback>();
    if (feedbackBuffer)
        fb->bind().setFeedbackBuffer(feedbackBuffer);
    return fb;
}

void TransformFeedback::BoundTransformFeedback::setFeedbackBuffer(const SharedBuffer &feedbackBuffer)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!feedback->isRecording(), "Cannot set feedback buffer while recoding " << to_string(feedback), return );

    feedback->mFeedbackBuffer = feedbackBuffer;
    checkValidGLOW();

    // bind buffer to feedback
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, feedbackBuffer->getObjectName());
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, feedbackBuffer->getObjectName());
}

void TransformFeedback::BoundTransformFeedback::begin(GLenum primitiveMode)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!feedback->isRecording(), "Cannot call begin when Feedback is still running " << to_string(feedback), return );

    Program::getCurrentProgram()->beginTransformFeedback(primitiveMode); // buffer already set
    feedback->mIsRecording = true;
}

void TransformFeedback::BoundTransformFeedback::end()
{
    if (!isCurrent())
        return;

    if (!feedback->isRecording())
        return;

    Program::getCurrentProgram()->endTransformFeedback();
    feedback->mIsRecording = false;
}

TransformFeedback::BoundTransformFeedback::BoundTransformFeedback(TransformFeedback *feedback) : feedback(feedback)
{
    checkValidGLOW();
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_BINDING, &previousFeedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback->getObjectName());

    previousFeedbackPtr = sCurrentFeedback;
    sCurrentFeedback = this;
}

bool TransformFeedback::BoundTransformFeedback::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentFeedback == this,
                        "Currently bound TransformFeedback does NOT match represented feedback " << to_string(feedback),
                        return false);
    return true;
}

TransformFeedback::BoundTransformFeedback::BoundTransformFeedback(TransformFeedback::BoundTransformFeedback &&rhs)
  : feedback(rhs.feedback), previousFeedback(rhs.previousFeedback), previousFeedbackPtr(rhs.previousFeedbackPtr)
{
    // invalidate rhs
    rhs.previousFeedback = -1;
}

TransformFeedback::BoundTransformFeedback::~BoundTransformFeedback()
{
    if (previousFeedback != -1) // if valid
    {
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, previousFeedback);
        sCurrentFeedback = previousFeedbackPtr;
    }
}
