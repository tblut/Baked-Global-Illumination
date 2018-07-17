#pragma once

#include "glow/common/non_copyable.hh"
#include "glow/common/property.hh"
#include "glow/common/shared.hh"

#include "glow/gl.hh"

#include "ColorSpace.hh"

#include <functional>
#include <string>
#include <vector>

namespace glow
{
GLOW_SHARED(class, TextureData);
GLOW_SHARED(class, SurfaceData);
/**
 * @brief Generic container for arbitrary texture data
 *
 * This class has no GPU dependency and can be saved to a file.
 * The restored texture has the same state as the original one.
 *
 * This class has few consistency checks so be sure to populate it correctly!
 *
 * If enums have the value GL_INVALID_ENUM, they are treated as "unspecified"
 *
 * See createFromFile for supported formats
 *
 * If you want to extend/overwrite loading behavior, use TextureData::setLoadFunction(...)
 */
class TextureData
{
    GLOW_NON_COPYABLE(TextureData);

public:
    /// Ending is always lower-case
    typedef std::function<SharedTextureData(std::string const& filename, std::string const& ending, ColorSpace colorSpace)> LoadFromFileFunc;

private:
    int mWidth = 0;
    int mHeight = 0;
    int mDepth = 0;

    GLenum mTarget = GL_INVALID_ENUM;

    GLenum mWrapS = GL_INVALID_ENUM;
    GLenum mWrapT = GL_INVALID_ENUM;
    GLenum mWrapR = GL_INVALID_ENUM;

    GLenum mMinFilter = GL_INVALID_ENUM;
    GLenum mMagFilter = GL_INVALID_ENUM;

    GLfloat mAnisotropicFiltering = -1.f;

    GLenum mPreferredInternalFormat = GL_INVALID_ENUM;

    /// List of data surfaces in this texture
    std::vector<SharedSurfaceData> mSurfaces;

    std::string mMetadata;

public: // getter, setter
    GLOW_PROPERTY(Width);
    GLOW_PROPERTY(Height);
    GLOW_PROPERTY(Depth);

    GLOW_PROPERTY(Target);

    GLOW_PROPERTY(WrapS);
    GLOW_PROPERTY(WrapT);
    GLOW_PROPERTY(WrapR);

    GLOW_PROPERTY(MinFilter);
    GLOW_PROPERTY(MagFilter);

    GLOW_PROPERTY(AnisotropicFiltering);

    GLOW_PROPERTY(PreferredInternalFormat);

    GLOW_GETTER(Surfaces);

    GLOW_PROPERTY(Metadata);

public:
    TextureData();

    /// Adds a surface layer to this texture
    /// Does not check if it conflicts with any other surface
    void addSurface(SharedSurfaceData const& surface);

public: // serialization
    /// Reads texture data from file
    /// Supported endings:
    ///  * via lodepng for 16bpc png
    ///     * PNG
    ///  * via stb_image
    ///     * JPEG
    ///     * TGA
    ///     * BMP
    ///     * PSD
    ///     * GIF
    ///     * HDR
    ///     * PIC
    ///     * PPM binary
    ///     * PGM binary
    ///  * via GLI
    ///     * KTX
    ///     * DDS
    ///  * via internal code
    ///     * .texdata (100% read/write features)
    ///
    /// NOTE: if you want to load a normalmap, you probably want ColorSpace::Linear!
    static SharedTextureData createFromFile(std::string const& filename, ColorSpace colorSpace);
    /// Creates texture data for a cubemap, give 6 files
    /// See "createFromFile" for format doc
    static SharedTextureData createFromFileCube(std::string const& fpx,
                                                std::string const& fnx,
                                                std::string const& fpy,
                                                std::string const& fny,
                                                std::string const& fpz,
                                                std::string const& fnz,
                                                ColorSpace colorSpace);
    /// Saves this data into a file
    /// Supported endings:
    ///  * via lodepng
    ///     * PNG
    ///  * via stb_image_write
    ///     * TGA
    ///     * BMP
    ///     * HDR
    ///  * via GLI
    ///     * KTX
    ///     * DDS
    ///  * via internal code
    ///     * .texdata (100% read/write features)
    ///
    /// Quality is in percent and not supported by all formats
    void saveToFile(std::string const& filename, int quality = 90);
    /// Sets the internally used function for loading files
    /// Set this to nullptr if you want to use the predefined one
    /// Return nullptr for everything you cannot load so the internal one may try to
    static void setLoadFunction(LoadFromFileFunc const& func);

private:
    /// Actual loader function
    static LoadFromFileFunc sLoadFunc;

    /// Loads with lodepng
    static SharedTextureData loadWithLodepng(std::string const& filename, std::string const& ending, ColorSpace colorSpace);
    /// Saves with lodepng
    static void saveWithLodepng(TextureData* data, std::string const& filename, std::string const& ending);

    /// Loads with stb
    static SharedTextureData loadWithStb(std::string const& filename, std::string const& ending, ColorSpace colorSpace);
    /// Saves with stb
    static void saveWithStb(TextureData* data, std::string const& filename, std::string const& ending);

    /// Loads with GLI
    static SharedTextureData loadWithGLI(std::string const& filename, std::string const& ending, ColorSpace colorSpace);
    /// Saves with GLI
    static void saveWithGLI(TextureData* data, std::string const& filename, std::string const& ending);

    /// Loads with custom binary
    static SharedTextureData loadWithBinary(std::string const& filename, std::string const& ending, ColorSpace colorSpace);
    /// Saves with custom binary
    static void saveWithBinary(TextureData* data, std::string const& filename, std::string const& ending);

#ifdef GLOW_USE_QT
    /// Loads with qt
    static SharedTextureData loadWithQt(std::string const& filename, std::string const& ending, ColorSpace colorSpace);
    /// Saves with qt
    static void saveWithQt(TextureData* data, std::string const& filename, std::string const& ending);
#endif
};
}
