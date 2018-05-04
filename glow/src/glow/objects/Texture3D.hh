// This file is auto-generated and should not be modified directly.
#pragma once

#include "Texture.hh"

#include "glow/common/gltypeinfo.hh"
#include "glow/common/warn_unused.hh"
#include "glow/common/log.hh"

#include "glow/data/ColorSpace.hh"

#include <vector>

#include <glm/vec4.hpp>

namespace glow
{
GLOW_SHARED(class, Texture3D);
GLOW_SHARED(class, TextureData);

/// Defines a 3D texture in OpenGL
class Texture3D final : public Texture
{
public:
    struct BoundTexture3D;

private:

    /// Minification filter
    GLenum mMinFilter = GL_NEAREST_MIPMAP_LINEAR;
    /// Magnification filter
    GLenum mMagFilter = GL_LINEAR;

    /// Border color
    glm::vec4 mBorderColor = {0.0f, 0.0f, 0.0f, 0.0f};

    /// Wrapping in S
    GLenum mWrapS = GL_REPEAT;
    /// Wrapping in T
    GLenum mWrapT = GL_REPEAT;
    /// Wrapping in R
    GLenum mWrapR = GL_REPEAT;


    /// Level of anisotropic filtering (>= 1.f, which is isotropic)
    /// Max number of samples basically
    GLfloat mAnisotropicFiltering = 1.0f;

    /// Texture size: Width
    int mWidth = 0u;
    /// Texture size: Height
    int mHeight = 0u;
    /// Texture size: Depth
    int mDepth = 0u;

    /// True iff mipmaps are generated since last data upload
    bool mMipmapsGenerated = false;

    /// LOD bias
    float mLodBias = 0.0f;
    /// Mipmap base level
    int mBaseLevel = 0;
    /// Mipmap max level
    int mMaxLevel = 1000;
    /// Mipmap min lod
    float mMinLod = -1000.0f;
    /// Mipmap max lod
    float mMaxLod = 1000.0f;

    /// if true, this texture got immutable storage by glTexStorage2D
    bool mStorageImmutable = false;

public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTexture3D* getCurrentTexture();

    GLenum getMinFilter() const { return mMinFilter; }
    GLenum getMagFilter() const { return mMagFilter; }
    glm::vec4 getBorderColor() const { return mBorderColor; }
    GLenum getWrapS() const { return mWrapS; }
    GLenum getWrapT() const { return mWrapT; }
    GLenum getWrapR() const { return mWrapR; }
    int getWidth() const { return mWidth; }
    int getHeight() const { return mHeight; }
    int getDepth() const { return mDepth; }
    glm::uvec3 getDimensions() const override { return { mWidth, mHeight, mDepth }; }

    bool isStorageImmutable() const override { return mStorageImmutable; }

    /// returns true iff mipmaps are used (based on min filter)
    bool hasMipmapsEnabled() const override;
    /// returns true iff mipmaps were generated via bind().generateMipmaps() (and are still valid)
    bool areMipmapsGenerated() const override { return mMipmapsGenerated; }
    /// Manually sets the internal flag if mipmaps were generated
    /// CAUTION: this should only be used if you modified the mipmap status manually (e.g. via glGenerateMipmaps)
    void setMipmapsGenerated(bool areGenerated) override { mMipmapsGenerated = areGenerated; }

    float getLodBias() const { return mLodBias; }
    int getBaseLevel() const { return mBaseLevel; }
    int getMaxLevel() const { return mMaxLevel; }
    float getMinLod() const { return mMinLod; }
    float getMaxLod() const { return mMaxLod; }

    /// returns the uniform type that should be used for sampling this texture
    GLenum getUniformType() const override;

public:
    /// RAII-object that defines a "bind"-scope for a 3D texture
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTexture3D
    {
        GLOW_RAII_CLASS(BoundTexture3D);

        /// Backreference to the texture
        Texture3D* const texture;

        /// Makes the storage of this texture immutable
        /// It is an error to call this more than once
        /// It is an error to upload data with a different internal format at a later point
        /// It is an error to resize after storage was made immutable (unless it's the same size)
        /// Invalidates previously uploaded data
        /// If mipmapLevels is <= 0, log2(max(width, height, depth)) + 1 is used
        void makeStorageImmutable(int width, int height, int depth, GLenum internalFormat, int mipmapLevels = 0);

        /// Sets minification filter (GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, ..., GL_LINEAR_MIPMAP_LINEAR)
        void setMinFilter(GLenum filter);
        /// Sets magnification filter (GL_NEAREST, GL_LINEAR)
        void setMagFilter(GLenum filter);
        /// Sets mag and min filter
        void setFilter(GLenum magFilter, GLenum minFilter);

        /// Sets the number of anisotropic samples (>= 1)
        void setAnisotropicFiltering(GLfloat samples);

        /// Sets the border color
        void setBorderColor(glm::vec4 const& color);

        /// Sets texture wrapping in S
        void setWrapS(GLenum wrap);
        /// Sets texture wrapping in T
        void setWrapT(GLenum wrap);
        /// Sets texture wrapping in R
        void setWrapR(GLenum wrap);
        /// Sets texture wrapping in all directions
        void setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR);


        /// Generates mipmaps for this texture
        void generateMipmaps();

        /// Sets the bias (offset) for LOD
        void setLodBias(float bias);
        /// Sets the finest uploaded mip level
        void setBaseLevel(int lvl);
        /// Sets the coarsest uploaded mip level
        void setMaxLevel(int lvl);
        /// Sets the smallest lod value that should be used by texture(...)
        void setMinLod(float lvl);
        /// Sets the largest lod value that should be used by texture(...)
        void setMaxLod(float lvl);

        /// Resizes the texture
        /// invalidates the data
        void resize(int width, int height, int depth);

        /// Generic data uploads
        /// Changes internal format, width, height, depth, and data
        void setData(GLenum internalFormat, int width, int height, int depth, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setData(GLenum internalFormat, int width, int height, int depth, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if ((int)data.size() != width * height * depth)
            {
                error() << "Texture size is " << width << " x " << height << " x " << depth << " = " << width * height * depth << " but " << data.size()
                        << " pixels are provided. " << to_string(texture);
                return;
            }
            setData(internalFormat, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setData(iFormat, width, height, depth, texData);
        template <typename DataT, std::size_t N>
        void setData(GLenum internalFormat, int width, int height, int depth, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width * height * depth)
            {
                error() << "Texture size is " << width << " x " << height << " x " << depth << " = " << width * height * depth << " but " << N
                        << " pixels are provided. " << to_string(texture);
                return;
            }
            setData(internalFormat, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [depth][height][width]
        ///   setData(iFormat, texData);
        template <typename DataT, int width, int height, int depth>
        void setData(GLenum internalFormat, const DataT(&data)[depth][height][width] , int mipmapLevel = 0)
        {
            setData(internalFormat, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }

        /// Generic partial data uploads
        /// Only changes data. Offset and size must be inside original bounds.
        void setSubData(int x, int y, int z, int width, int height, int depth, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Partial data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setSubData(int x, int y, int z, int width, int height, int depth, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if ((int)data.size() != width * height * depth)
            {
                error() << "Texture size is " << width << " x " << height << " x " << depth << " = " << width * height * depth << " but " << data.size()
                        << " pixels are provided. " << to_string(texture);
                return;
            }
            setSubData(x, y, z, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setSubData(x, y, z, width, height, depth, texData);
        template <typename DataT, std::size_t N>
        void setSubData(int x, int y, int z, int width, int height, int depth, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width * height * depth)
            {
                error() << "Texture size is " << width << " x " << height << " x " << depth << " = " << width * height * depth << " but " << N
                        << " pixels are provided. " << to_string(texture);
                return;
            }
            setSubData(x, y, z, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [depth][height][width]
        ///   setSubData(x, y, z, texData);
        template <typename DataT, int width, int height, int depth>
        void setSubData(int x, int y, int z, const DataT(&data)[depth][height][width] , int mipmapLevel = 0)
        {
            setSubData(x, y, z, width, height, depth, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }

        /// Sets texture data from surface data
        /// May set multiple levels at once
        /// May modify texture parameter
        void setData(GLenum internalFormat, SharedTextureData const& data);

        /// Generic data download
        std::vector<char> getData(GLenum format, GLenum type, int mipmapLevel = 0);
        /// Generic data download
        void getData(GLenum format, GLenum type, size_t bufferSize, void* buffer, int mipmapLevel = 0);
        /// Data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getData(int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(texture->mWidth * texture->mHeight * texture->mDepth);
            getData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.size() * sizeof(DataT), data.data(), mipmapLevel);
            return std::move(data);
        }

        /* TODO: OpenGL <4.5 does not support subimage retrieval (in 4.5, https://www.opengl.org/sdk/docs/man/html/glGetTextureSubImage.xhtml can be used)
        /// Generic partial data download
        std::vector<char> getSubData(GLenum format, GLenum type, int x, int y, int z, int width, int height, int depth, int mipmapLevel = 0);
        /// Generic partial data download
        void getSubData(GLenum format, GLenum type, int x, int y, int z, int width, int height, int depth, size_t bufferSize, void* buffer, int mipmapLevel = 0);
        /// Partial data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getSubData(int x, int y, int z, int width, int height, int depth, int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(width * height * depth);
            getSubData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, x, y, z, width, height, depth, data.size() * sizeof(DataT), data.data(), mipmapLevel);
            return std::move(data);
        }
        */

        /// Extracts all stored surface data up to a given max mipmap level (inclusive)
        /// This is useful for saving the texture to a file
        SharedTextureData getTextureData(int maxMipLevel = 1000);
        /// Same as getTextureData()->writeToFile(filename)
        void writeToFile(std::string const& filename);

    private:
        GLint previousTexture;              ///< previously bound tex
        BoundTexture3D* previousTexturePtr; ///< previously bound tex
        BoundTexture3D (Texture3D* buffer);
        friend class Texture3D;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTexture3D (BoundTexture3D &&); // allow move
        ~BoundTexture3D ();
    };
public:

    /// Fills the specific mipmap level (default 0) with the given data
    /// Requires OpenGL 4.4 (for now) and will throw a run-time error otherwise
    void clear(GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
    /// Clear via glm or c++ type (see gltypeinfo)
    /// CAREFUL: pointers do not work!
    template <typename DataT>
    void clear(DataT const& data, int mipmapLevel = 0)
    {
        clear(glTypeOf<DataT>::format, glTypeOf<DataT>::type, (const GLvoid*)&data, mipmapLevel);
    }

public:
    Texture3D (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTexture3D bind() { return {this}; }
public: // static construction
    /// Creates a 3D texture with given width and height
    static SharedTexture3D create(int width = 1, int height = 1, int depth = 1, GLenum internalFormat = GL_RGBA);
    /// Creates a 3D texture with given width and height which is storage immutable
    /// If mipmapLevels is <= 0, log2(max(width, height)) + 1 is used
    static SharedTexture3D createStorageImmutable(int width, int height, int depth, GLenum internalFormat, int mipmapLevels = 0);

    /// Creates a 3D texture from file
    /// See TextureData::createFromFile for format documentation
    /// Uses preferred internal format
    static SharedTexture3D createFromFile(std::string const& filename, ColorSpace colorSpace);
    /// same as createFromFile but with custom internal format
    static SharedTexture3D createFromFile(std::string const& filename, GLenum internalFormat, ColorSpace colorSpace);

    /// Creates a 3D texture from given data
    /// Uses preferred internal format
    static SharedTexture3D createFromData(SharedTextureData const& data);
    /// same as createFromData but with custom internal format
    static SharedTexture3D createFromData(SharedTextureData const& data, GLenum internalFormat);

    friend class Framebuffer;
};
}
