#pragma once

#include "glow/common/non_copyable.hh"
#include "glow/common/shared.hh"

#include "glow/gl.hh"

#include "NamedObject.hh"

namespace glow
{
GLOW_SHARED(class, Buffer);

class Buffer : public NamedObject<Buffer, GL_BUFFER>
{
    GLOW_NON_COPYABLE(Buffer);

    /// OGL id
    GLuint mObjectName;

    /// Buffer type
    GLenum mType;

    /// Reference to original, unaliased buffer
    SharedBuffer mOriginalBuffer;

public: // getter
    GLuint getObjectName() const { return mObjectName; }
    GLenum getType() const { return mType; }

    /// if true, this buffer shares GPU memory with the original buffer (which might be of different type)
    bool isAliasedBuffer() const { return mOriginalBuffer != nullptr; }
    /// if "isAliasedBuffer", returns the original buffer (guaranteed !isAliasedBuffer)
    SharedBuffer getOriginalBuffer() const { return mOriginalBuffer; }

protected:
    /// creates a new buffer
    /// if originalBuffer is not null, this creates an aliased buffer
    Buffer(GLenum bufferType, SharedBuffer const& originalBuffer = nullptr);
    virtual ~Buffer();
};
}
