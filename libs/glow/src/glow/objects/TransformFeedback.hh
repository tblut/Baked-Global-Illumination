#pragma once

#include <glow/common/shared.hh>
#include <glow/common/non_copyable.hh>
#include <glow/common/warn_unused.hh>

#include "glow/gl.hh"

#include "NamedObject.hh"

namespace glow
{
GLOW_SHARED(class, Buffer);
GLOW_SHARED(class, TransformFeedback);

/**
 * @brief A TransformFeedback object encapsulates and manages state for Transform Feedback
 *
 * This includes:
 *   - The actual feedbacks used for feedback
 *   - Recorded primitive count
 *
 * Usage:
 *   - init:
 *       program->configureTransformFeedback(...)
 *       feedback = TransformFeedback::create(buffer);
 *   - rendering:
 *       auto shader = program->use();
 *       myVAO->bind().negotiateBindings(); // important bcz relinking cannot happen during transform feedback
 *       {
 *         auto fb = feedback->bind();
 *         fb->begin();
 *         ... render myVAO
 *         fb->end();
 *       }
 *       otherVAO->bind().drawTransformFeedback(feedback);
 */
class TransformFeedback : public NamedObject<TransformFeedback, GL_TRANSFORM_FEEDBACK>
{
    GLOW_NON_COPYABLE(TransformFeedback);

public:
    struct BoundTransformFeedback;

private:
    /// OGL id
    GLuint mObjectName;

    /// True iff currently recording
    bool mIsRecording = false;

    /// Buffer bound to GL_TRANSFORM_FEEDBACK_BUFFER
    /// Can only be set while bount (and not recording)
    SharedBuffer mFeedbackBuffer;

public: // getter
    /// Gets the currently bound TransformFeedback (nullptr if none)
    static BoundTransformFeedback* getCurrentFeedback();

    GLuint getObjectName() const { return mObjectName; }
    bool isRecording() const { return mIsRecording; }
    bool isBound() const { return getCurrentFeedback() != nullptr && getCurrentFeedback()->feedback == this; }
    SharedBuffer const& getFeedbackBuffer() const { return mFeedbackBuffer; }
public:
    /// RAII-object that defines a "bind"-scope for an TransformFeedback
    /// All functions that operate on the currently bound object are accessed here
    struct BoundTransformFeedback
    {
        GLOW_RAII_CLASS(BoundTransformFeedback);

        /// Backreference to the feedback
        TransformFeedback* const feedback;

        /// Sets and binds a new feedback buffer (not valid while recording)
        void setFeedbackBuffer(SharedBuffer const& feedbackBuffer);

        /// Starts the transform feedback (no nesting allowed!)
        void begin(GLenum primitiveMode);
        /// Ends the transform feedback (no-op if not recording)
        void end();

    private:
        GLint previousFeedback;                      ///< previously bound feedback
        BoundTransformFeedback* previousFeedbackPtr; ///< previously bound feedback
        BoundTransformFeedback(TransformFeedback* feedback);
        friend class TransformFeedback;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTransformFeedback(BoundTransformFeedback&&); // allow move
        ~BoundTransformFeedback();
    };

public:
    /// Creates a new TransformFeedback object
    TransformFeedback();
    ~TransformFeedback();

    /// Binds this transform feedback.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTransformFeedback bind() { return {this}; }
public:
    /// Creates a new TransformFeedback
    static SharedTransformFeedback create(SharedBuffer const& feedbackBuffer = nullptr);
};
}
