#pragma once

#include "VertexArrayAttribute.hh"

#include "glow/common/non_copyable.hh"
#include "glow/common/shared.hh"
#include "glow/common/warn_unused.hh"

#include "glow/gl.hh"

#include "NamedObject.hh"

#include <vector>

namespace glow
{
GLOW_SHARED(class, ArrayBuffer);
GLOW_SHARED(class, ElementArrayBuffer);
GLOW_SHARED(class, VertexArray);
GLOW_SHARED(class, LocationMapping);
GLOW_SHARED(class, TransformFeedback);

class VertexArray final : public NamedObject<VertexArray, GL_VERTEX_ARRAY>
{
    GLOW_NON_COPYABLE(VertexArray);

public:
    struct BoundVertexArray;

private:
    /// OGL id
    GLuint mObjectName;

    /// OGL primitive mode
    GLenum mPrimitiveMode;
    /// Number of vertices per patch (for primitive mode GL_PATCHES)
    int mVerticesPerPatch = -1;

    /// Attached element array buffer
    SharedElementArrayBuffer mElementArrayBuffer;

    /// ArrayBuffer Attributes
    std::vector<VertexArrayAttribute> mAttributes;

    /// Location mapping from attribute name to loc
    /// At any point, the mapping saved here must be consistent (i.e. a superset) of the GPU mapping
    SharedLocationMapping mAttributeMapping;

private:
    /// Attaches the given attribute to the current VAO
    static void attachAttribute(VertexArrayAttribute const& a);

public: // getter
    GLuint getObjectName() const { return mObjectName; }
    GLenum getPrimitiveMode() const { return mPrimitiveMode; }
    void setPrimitiveMode(GLenum mode) { mPrimitiveMode = mode; }
    void setVerticesPerPatch(int cnt) { mVerticesPerPatch = cnt; }
    SharedLocationMapping const& getAttributeMapping() const { return mAttributeMapping; }
    SharedElementArrayBuffer const& getElementArrayBuffer() const { return mElementArrayBuffer; }
    std::vector<VertexArrayAttribute> const& getAttributes() const { return mAttributes; }

    /// returns true iff a draw call would draw zero primitives
    bool isEmpty() const;

public:
    /// returns the AB that contains the given attribute (or nullptr if not found)
    SharedArrayBuffer getAttributeBuffer(std::string const& name) const;
    /// returns the number of instances that this VertexArray should logically draw
    /// is 1 if no divisor > 0 is present
    /// otherwise max of divisor * elementCount per attribute
    int getInstanceCount() const;
    /// returns the number of primitives in the first divisor-0 array buffer
    /// returns 0 if none present
    int getVertexCount() const;

public:
    /// Gets the currently bound VAO (nullptr if none)
    static BoundVertexArray* getCurrentVAO();

public:
    /// RAII-object that defines a "bind"-scope for a VertexArray
    /// All functions that operate on the currently bound VAO are accessed here
    struct BoundVertexArray
    {
        GLOW_RAII_CLASS(BoundVertexArray);

        /// Backreference to the program
        VertexArray* const vao;

    public: // gl function that require binding
        /// Draws the VAO
        /// Requires a currently used shader (otherwise: runtime error)
        /// Automatically determines index and unindexed drawing
        /// Uses the first divisor-0 array buffer to determine number of primitives
        /// Negotiates location mappings IF current program != nullptr
        /// If instanceCount is negative, getInstanceCount() is used.
        void draw(GLsizei instanceCount = -1);
        /// Same as draw(...) but only renders a subrange of indices or vertices
        void drawRange(GLsizei start, GLsizei end, GLsizei instanceCount = -1);
        /// Same as draw(...) but takes the number of vertices from a recorded transform feedback object
        /// NOTE: does not work with index buffers or instancing
        void drawTransformFeedback(SharedTransformFeedback const& feedback);

        /// Attaches an element array buffer
        /// Overrides the previously attached one
        /// nullptr is a valid argument
        void attach(SharedElementArrayBuffer const& eab);

        /// Attaches all attributes of the given array buffer
        /// Override attributes with the same name
        /// nullptr is NOT a valid argument
        void attach(SharedArrayBuffer const& ab);

        /// Attaches all attributes of the given array buffers
        /// Override attributes with the same name
        void attach(std::vector<SharedArrayBuffer> const& abs);

        /// Re-attaches array buffers with current locations
        /// Probably not required by an end-user
        void reattach();

        /// Negotiates attribute bindings with the currently bound program
        /// Is called automatically on use
        /// The only time you need this manually is when using transform feedback
        void negotiateBindings();

    private:
        GLint previousVAO;                ///< previously used vao
        GLint previousEAB;                ///< previously used eab
        BoundVertexArray* previousVaoPtr; ///< previously used vao
        BoundVertexArray(VertexArray* vao);
        friend class VertexArray;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

        /// updates patch parameters if GL_PATCHES is used
        void updatePatchParameters();

    public:
        BoundVertexArray(BoundVertexArray&&); // allow move
        ~BoundVertexArray();
    };

public:
    VertexArray(GLenum primitiveMode = GL_TRIANGLES);
    ~VertexArray();


    /// Binds this vertex array.
    /// Unbinding is done when the returned object runs out of scope.
    /// CAUTION: Cannot be used while an EAB is bound! (runtime error)
    GLOW_WARN_UNUSED BoundVertexArray bind() { return {this}; }

public: // static construction
    /// creates an empty VAO
    /// same as std::make_shared<VertexArray>()
    static SharedVertexArray create(GLenum primitiveMode = GL_TRIANGLES);
    /// creates a VAO and attaches all specified attributes (and optionally an EAB)
    static SharedVertexArray create(SharedArrayBuffer const& ab, SharedElementArrayBuffer const& eab, GLenum primitiveMode = GL_TRIANGLES);
    static SharedVertexArray create(SharedArrayBuffer const& ab, GLenum primitiveMode = GL_TRIANGLES);
    /// creates a VAO and attaches all specified attributes (and optionally an EAB)
    static SharedVertexArray create(std::vector<SharedArrayBuffer> const& abs,
                                    SharedElementArrayBuffer const& eab = nullptr,
                                    GLenum primitiveMode = GL_TRIANGLES);
};
}
