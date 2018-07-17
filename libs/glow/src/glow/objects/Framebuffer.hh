#pragma once

#include "glow/common/non_copyable.hh"
#include "glow/common/shared.hh"
#include "glow/common/warn_unused.hh"

#include "glow/gl.hh"

#include "NamedObject.hh"

#include <array>
#include <string>
#include <vector>

namespace glow
{
GLOW_SHARED(class, Framebuffer);
GLOW_SHARED(class, LocationMapping);
GLOW_SHARED(class, Texture);

/**
 * @brief The Framebuffer class
 *
 * For CubeMaps, layer = 0 (GL_TEXTURE_CUBE_MAP_POSITIVE_X) .. 5 (GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
 * For CubeMap Arrays, layer is (array index * 6 + cubemap layer)
 */
class Framebuffer final : public NamedObject<Framebuffer, GL_FRAMEBUFFER>
{
    GLOW_NON_COPYABLE(Framebuffer);

public:
    struct BoundFramebuffer;

    /// Color, depth, or stencil attachment
    struct Attachment
    {
        std::string locationName; ///< ignored for non-color
        SharedTexture texture;
        int mipmapLevel = 0;
        int layer = 0; ///< only valid for nD-array and cubemaps
        // TODO: support for "image"

        Attachment() = default;
        Attachment(std::string const& loc, SharedTexture const& tex, int miplevel = 0, int layer = 0)
          : locationName(loc), texture(tex), mipmapLevel(miplevel), layer(layer)
        {
        }
    };

private:
    /// OGL id
    GLuint mObjectName;

    /// Location mapping from fragment output name to loc
    /// At any point, the mapping saved here must be consistent (i.e. a superset) of the GPU mapping
    SharedLocationMapping mFragmentMapping;
    friend class VertexArray; // for negotiation

    /// List of color a.
    std::vector<Attachment> mColorAttachments;
    /// Current depth attachment
    Attachment mDepthAttachment = {"", nullptr, -1, -1};
    /// Current stencil attachment
    Attachment mStencilAttachment = {"", nullptr, -1, -1};

    /// if true, sets the viewport automatically to the minimal attached size
    bool mAutoViewport = true;

private:
    /// Careful! must be bound
    void internalReattach();
    /// Careful! must be bound
    bool internalCheckComplete();

public: // getter
    GLuint getObjectName() const { return mObjectName; }
    SharedLocationMapping const& getFragmentMapping() const { return mFragmentMapping; }
    std::vector<Attachment> const& getColorAttachments() const { return mColorAttachments; }
    Attachment const& getDepthAttachment() const { return mDepthAttachment; }
    Attachment const& getStencilAttachment() const { return mStencilAttachment; }
    void setAutoViewport(bool enable) { mAutoViewport = enable; }
    bool getAutoViewport() const { return mAutoViewport; }
    /// Gets the currently bound FBO (nullptr if none)
    static BoundFramebuffer* getCurrentBuffer();

public:
    /// Notifies this FBO that some shader wrote to it
    /// E.g. used to tell textures that their mipmaps are invalid
    void notifyShaderExecuted();

public:
    /// RAII-object that defines a "bind"-scope for a Framebuffer
    /// All functions that operate on the currently bound buffer are accessed here
    struct BoundFramebuffer
    {
        GLOW_RAII_CLASS(BoundFramebuffer);

        /// Backreference to the buffer
        Framebuffer* const buffer;

        /// Check if the FBO is complete
        /// returns true iff complete
        bool checkComplete();

        /// Reattaches all FBO attachments
        void reattach();

        /// Attaches a texture to a named fragment location (color output)
        /// Overrides previously bound textures to the same name
        /// nullptr is a valid texture
        void attachColor(std::string const& fragName, SharedTexture const& tex, int mipmapLevel = 0, int layer = 0);
        void attachColor(Attachment const& a);
        /// Attaches a texture to the depth target
        /// nullptr is a valid texture
        void attachDepth(SharedTexture const& tex, int mipmapLevel = 0, int layer = 0);
        /// Attaches a texture to the stencil target
        /// nullptr is a valid texture
        void attachStencil(SharedTexture const& tex, int mipmapLevel = 0, int layer = 0);
        /// Attaches a texture to the depth AND stencil target
        /// nullptr is a valid texture
        void attachDepthStencil(SharedTexture const& tex, int mipmapLevel = 0, int layer = 0);
        /// Creates a new depth texture with given format and attaches it
        /// Requires already attached color textures! (for size)
        /// May bind an texture
        void attachDepth(GLenum depthFormat = GL_DEPTH_COMPONENT32);

    private:
        GLint previousBuffer;                      ///< previously bound buffer
        std::array<GLenum, 8> previousDrawBuffers; ///< previously setup draw buffers
        std::array<GLint, 4> previousViewport;     ///< previously configure viewport
        BoundFramebuffer* previousBufferPtr;       ///< previously bound buffer
        BoundFramebuffer(Framebuffer* buffer);
        friend class Framebuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundFramebuffer(BoundFramebuffer&&); // allow move
        ~BoundFramebuffer();
    };

public:
    Framebuffer();
    ~Framebuffer();

    /// Binds this framebuffer.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundFramebuffer bind() { return {this}; }

public: // static construction
    /// Creates an empty framebuffer
    /// Same as std::make_shared<Framebuffer>();
    static SharedFramebuffer create();
    /// Creates a framebuffer with given color, depth, and stencil attachements
    /// The order of colors matches the target indices (e.g. for glClearBuffer or GL_DRAW_BUFFERi)
    /// CAUTION: This framebuffer is checked for completeness!
    static SharedFramebuffer create(std::vector<Attachment> const& colors,
                                    SharedTexture const& depth = nullptr,
                                    SharedTexture const& stencil = nullptr,
                                    int depthStencilMipmapLevel = 0,
                                    int depthStencilLayer = 0);
    /// Shortcut for create({{fragmentName, color}}, ...)
    static SharedFramebuffer create(std::string const& fragmentName,
                                    SharedTexture const& color,
                                    SharedTexture const& depth = nullptr,
                                    SharedTexture const& stencil = nullptr,
                                    int depthStencilMipmapLevel = 0,
                                    int depthStencilLayer = 0);
    /// Creates a framebuffer without attached colors
    /// CAUTION: This framebuffer is checked for completeness!
    static SharedFramebuffer createDepthOnly(SharedTexture const& depth,
                                             SharedTexture const& stencil = nullptr,
                                             int depthStencilMipmapLevel = 0,
                                             int depthStencilLayer = 0);
};
}
