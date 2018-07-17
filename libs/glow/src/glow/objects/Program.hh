#pragma once

#include "glow/common/gltypeinfo.hh"
#include "glow/common/macro_join.hh"
#include "glow/common/non_copyable.hh"
#include "glow/common/property.hh"
#include "glow/common/shared.hh"
#include "glow/common/warn_unused.hh"

#include "glow/gl.hh"

#include "glow/util/LocationMapping.hh"

#include "NamedObject.hh"

#include <map>
#include <string>
#include <vector>

// only vec/mat types
#include <glm/matrix.hpp>

namespace glow
{
GLOW_SHARED(class, Shader);
GLOW_SHARED(class, Program);
GLOW_SHARED(class, LocationMapping);
GLOW_SHARED(class, UniformState);
GLOW_SHARED(class, Texture);
GLOW_SHARED(class, Texture2D);
GLOW_SHARED(class, UniformBuffer);
GLOW_SHARED(class, ShaderStorageBuffer);
GLOW_SHARED(class, AtomicCounterBuffer);
GLOW_SHARED(class, Buffer);

class Program final : public NamedObject<Program, GL_PROGRAM>
{
    GLOW_NON_COPYABLE(Program);


public:
    struct UniformInfo
    {
        std::string name;
        GLint location = -1;
        GLint size = -1;
        GLenum type = GL_INVALID_ENUM;
        bool wasSet = false;
    };

    struct UsedProgram;

private:
    /// If true, shader check for shader reloading periodically
    static bool sCheckShaderReloading;


    /// OpenGL object name
    GLuint mObjectName;

    /// List of attached shader
    std::vector<SharedShader> mShader;
    /// Last time of reload checking
    int64_t mLastReloadCheck = 0;

    /// Uniform lookup cache
    /// Invalidates after link
    mutable std::vector<UniformInfo> mUniformCache;

    /// Texture unit mapping
    LocationMapping mTextureUnitMapping;
    /// Bounds texture (idx = unit)
    std::vector<SharedTexture> mTextures;

    /// Locations for in-VS-attributes
    /// At any point, the mapping saved here must be consistent (i.e. a superset) of the GPU mapping
    SharedLocationMapping mAttributeMapping;
    /// Locations for out-FS-attributes
    /// At any point, the mapping saved here must be consistent (i.e. a superset) of the GPU mapping
    SharedLocationMapping mFragmentMapping;
    /// Friend for negotiating mAttributeMapping
    friend class VertexArray;
    /// Friend for negotiating mFragmentMapping
    friend class FramebufferObject;

    /// Mapping for uniform buffers
    LocationMapping mUniformBufferMapping;
    /// Bound uniform buffer
    std::map<std::string, SharedUniformBuffer> mUniformBuffers;

    /// Mapping for shaderStorage buffers
    LocationMapping mShaderStorageBufferMapping;
    /// Bound shaderStorage buffer
    std::map<std::string, SharedShaderStorageBuffer> mShaderStorageBuffers;

    /// Bound AtomicCounter buffer
    std::map<int, SharedAtomicCounterBuffer> mAtomicCounterBuffers;

    /// List of varyings for transform feedback
    std::vector<std::string> mTransformFeedbackVaryings;
    /// Buffer mode for transform feedback
    GLenum mTransformFeedbackMode;
    /// if true, this program is linked properly for transform feedback
    bool mIsLinkedForTransformFeedback = false;

    /// true if already checked for layout(loc)
    bool mCheckedForAttributeLocationLayout = false;
    /// true if already checked for layout(loc)
    bool mCheckedForFragmentLocationLayout = false;

    /// disables unchanged uniform warning
    bool mWarnOnUnchangedUniforms = true;
    /// true if already checked for unchanged uniforms
    bool mCheckedUnchangedUniforms = false;

private:
    /// Internal linking
    /// returns false on errors
    bool linkAndCheckErrors();

    /// Restores additional "program state", like textures and shader buffers
    void restoreExtendedState();

    /// Reset frag location check
    /// Necessary because frags cannot be queried ...
    void resetFragmentLocationCheck() { mCheckedForFragmentLocationLayout = false; }

    /// see public: version
    UniformInfo* getUniformInfo(std::string const& name);

    /// returns uniform location and sets "wasSet" to true
    /// Also verifies that the uniform types match
    GLint useUniformLocationAndVerify(std::string const& name, GLint size, GLenum type);

public: // properties
    GLuint getObjectName() const { return mObjectName; }
    std::vector<SharedShader> const& getShader() const { return mShader; }
    SharedLocationMapping const& getAttributeMapping() const { return mAttributeMapping; }
    SharedLocationMapping const& getFragmentMapping() const { return mFragmentMapping; }
    LocationMapping const& getTextureUnitMapping() const { return mTextureUnitMapping; }

    GLOW_PROPERTY(WarnOnUnchangedUniforms);

    /// returns true iff a shader with this type is attached
    bool hasShaderType(GLenum type) const;

    /// Gets the currently used program (nullptr if none)
    static UsedProgram* getCurrentProgram();

    /// Modifies shader reloading state
    static void setShaderReloading(bool enabled);

public:
    /// Checks if all bound textures have valid mipmaps
    void validateTextureMipmaps() const;

    /// if not already done so:
    /// checks if any uniform is used but not set
    void checkUnchangedUniforms();

public:
    /// RAII-object that defines a "use"-scope for a Program
    /// All functions that operate on the currently bound program are accessed here
    struct UsedProgram
    {
        GLOW_RAII_CLASS(UsedProgram);

        /// Backreference to the program
        Program* const program;

    public: // gl functions with use
        /// Binds a texture to a uniform
        /// Automatically chooses a free texture unit starting from 0
        /// Setting nullptr is ok
        void setTexture(std::string const& name, SharedTexture const& tex);
        /// Binds a texture to an image sampler
        /// Requires an explicit binding location in shader (e.g. binding=N layout)
        /// Requires tex->isStorageImmutable()
        void setImage(int bindingLocation, SharedTexture const& tex, GLenum usage = GL_READ_WRITE, int mipmapLevel = 0, int layer = 0);

        /// ========================================= UNIFORMS - START =========================================
        /// This section defines the various ways of setting uniforms
        /// Setting by uniform _location_ is explicitly NOT supported because drawing may relink and thus reshuffle
        /// locations. However, locations are internally cached, so the performance hit is not high.
        ///
        /// Supported variants:
        ///   setUniform(string name, T value)
        ///   setUniform(string name, int count, T* values)
        ///   setUniform(string name, vector<T> values)
        ///   setUniform(string name, T values[N])
        ///   setUniform(string name, std::array<T, N> values)
        ///   setUniform(string name, {v0, v1, v2, ...})
        ///
        /// for T in
        ///   bool
        ///   int
        ///   float
        ///   unsigned
        ///   [ iub]vec[234]
        ///   mat[234]
        ///   mat[234]x[234]
        ///
        /// NOTE: there are no double uniforms!
        ///
        /// Using the wrong type results in a runtime error.
        /// TODO: make this configurable

        /// Generic interface
        void setUniform(std::string const& name, int count, bool const* values) const;
        void setUniform(std::string const& name, int count, int32_t const* values) const;
        void setUniform(std::string const& name, int count, uint32_t const* values) const;
        void setUniform(std::string const& name, int count, float const* values) const;

        void setUniform(std::string const& name, int count, glm::vec2 const* values) const;
        void setUniform(std::string const& name, int count, glm::vec3 const* values) const;
        void setUniform(std::string const& name, int count, glm::vec4 const* values) const;
        void setUniform(std::string const& name, int count, glm::ivec2 const* values) const;
        void setUniform(std::string const& name, int count, glm::ivec3 const* values) const;
        void setUniform(std::string const& name, int count, glm::ivec4 const* values) const;
        void setUniform(std::string const& name, int count, glm::uvec2 const* values) const;
        void setUniform(std::string const& name, int count, glm::uvec3 const* values) const;
        void setUniform(std::string const& name, int count, glm::uvec4 const* values) const;
        void setUniform(std::string const& name, int count, glm::bvec2 const* values) const;
        void setUniform(std::string const& name, int count, glm::bvec3 const* values) const;
        void setUniform(std::string const& name, int count, glm::bvec4 const* values) const;

        void setUniform(std::string const& name, int count, glm::mat2x2 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat2x3 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat2x4 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat3x2 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat3x3 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat3x4 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat4x2 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat4x3 const* values) const;
        void setUniform(std::string const& name, int count, glm::mat4x4 const* values) const;

/// Convenience interfaces
#define GLOW_PROGRAM_UNIFORM_API_NO_VEC(TYPE)                                                   \
    void setUniform(std::string const& name, TYPE value) const { setUniform(name, 1, &value); } \
    template <std::size_t N>                                                                    \
    void setUniform(std::string const& name, const TYPE(&data)[N])                              \
    {                                                                                           \
        setUniform(name, N, data);                                                              \
    }                                                                                           \
    template <std::size_t N>                                                                    \
    void setUniform(std::string const& name, std::array<TYPE, N> const& data)                   \
    {                                                                                           \
        setUniform(name, N, data.data());                                                       \
    }                                                                                           \
    void setUniform(std::string const& name, std::initializer_list<TYPE> const& data)           \
    {                                                                                           \
        setUniform(name, data.size(), data.begin());                                            \
    }                                                                                           \
    friend class GLOW_MACRO_JOIN(___prog_uniform_api_no_vec_, __COUNTER__) // enfore ;
#define GLOW_PROGRAM_UNIFORM_API(TYPE)                                              \
    GLOW_PROGRAM_UNIFORM_API_NO_VEC(TYPE);                                          \
    void setUniform(std::string const& name, std::vector<TYPE> const& values) const \
    {                                                                               \
        setUniform(name, values.size(), values.data());                             \
    }                                                                               \
    friend class GLOW_MACRO_JOIN(___prog_uniform_api_, __COUNTER__) // enfore ;

        // basis types
        GLOW_PROGRAM_UNIFORM_API_NO_VEC(bool);
        GLOW_PROGRAM_UNIFORM_API(int32_t);
        GLOW_PROGRAM_UNIFORM_API(uint32_t);
        GLOW_PROGRAM_UNIFORM_API(float);

        // vector types
        GLOW_PROGRAM_UNIFORM_API(glm::vec2);
        GLOW_PROGRAM_UNIFORM_API(glm::vec3);
        GLOW_PROGRAM_UNIFORM_API(glm::vec4);
        GLOW_PROGRAM_UNIFORM_API(glm::ivec2);
        GLOW_PROGRAM_UNIFORM_API(glm::ivec3);
        GLOW_PROGRAM_UNIFORM_API(glm::ivec4);
        GLOW_PROGRAM_UNIFORM_API(glm::uvec2);
        GLOW_PROGRAM_UNIFORM_API(glm::uvec3);
        GLOW_PROGRAM_UNIFORM_API(glm::uvec4);
        GLOW_PROGRAM_UNIFORM_API(glm::bvec2);
        GLOW_PROGRAM_UNIFORM_API(glm::bvec3);
        GLOW_PROGRAM_UNIFORM_API(glm::bvec4);

        // matrix types
        GLOW_PROGRAM_UNIFORM_API(glm::mat2x2);
        GLOW_PROGRAM_UNIFORM_API(glm::mat2x3);
        GLOW_PROGRAM_UNIFORM_API(glm::mat2x4);
        GLOW_PROGRAM_UNIFORM_API(glm::mat3x2);
        GLOW_PROGRAM_UNIFORM_API(glm::mat3x3);
        GLOW_PROGRAM_UNIFORM_API(glm::mat3x4);
        GLOW_PROGRAM_UNIFORM_API(glm::mat4x2);
        GLOW_PROGRAM_UNIFORM_API(glm::mat4x3);
        GLOW_PROGRAM_UNIFORM_API(glm::mat4x4);

        /// Special case: vector<bool>
        void setUniform(std::string const& name, std::vector<bool> const& values) const
        {
            auto cnt = values.size();
            std::vector<int32_t> tmp(cnt);
            for (auto i = 0u; i < cnt; ++i)
                tmp[i] = (int)values[i];
            setUniformBool(name, tmp.size(), tmp.data());
        }

        /// Applies all saved uniforms from that state
        void setUniforms(SharedUniformState const& state);
        /// Sets generic uniform data
        /// CAUTION: bool is assumed as glUniformi (i.e. integer)
        void setUniform(std::string const& name, GLenum uniformType, GLint size, void const* data);
        /// ========================================== UNIFORMS - END ==========================================


        /// Invokes the compute shader with the given number of groups
        void compute(GLuint groupsX = 1, GLuint groupsY = 1, GLuint groupsZ = 1);

        /// Starts transform feedback
        /// feedbackBuffer is an optional buffer that is automatically bound to GL_TRANSFORM_FEEDBACK_BUFFER
        /// NOTE: it is recommended to use a FeedbackObject
        void beginTransformFeedback(GLenum primitiveMode, SharedBuffer const& feedbackBuffer = nullptr);
        /// Ends transform feedback
        void endTransformFeedback();

    private:
        /// Special case: bool uniforms
        void setUniformBool(std::string const& name, int count, int32_t const* values) const;

        /// Special case: overwrite uniform type
        void setUniformIntInternal(std::string const& name, int count, int32_t const* values, GLenum uniformType) const;

        /// Check for shader reloading
        void checkShaderReload();

    private:
        GLint previousProgram;           ///< previously used program
        UsedProgram* previousProgramPtr; ///< previously used program
        UsedProgram(Program* program);
        friend class Program;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        UsedProgram(UsedProgram&&); // allow move
        ~UsedProgram();
    };

public: // gl functions without use
    /// Returns the in-shader location of the given uniform name
    /// Is -1 if not found or optimized out (!)
    /// Uses an internal cache to speedup the call
    GLint getUniformLocation(std::string const& name) const;

    /// Returns information about a uniform
    /// If location is -1, information is nullptr
    /// (Returned pointer is invalidated if shader is relinked)
    UniformInfo const* getUniformInfo(std::string const& name) const;

    /// Returns the index of a uniform block
    GLuint getUniformBlockIndex(std::string const& name) const;

    /// ========================================= UNIFORMS - START =========================================
    /// Getter for uniforms
    /// If you get linker errors, the type is not supported
    /// Returns default-constructed values for optimized uniforms
    /// Usage:
    ///    auto pos = prog->getUniform<glm::vec3>("uPosition");
    ///
    /// LIMITATIONS:
    /// currently doesn't work for arrays/structs
    ///
    /// Supported types:
    ///  * bool
    ///  * float
    ///  * int
    ///  * unsigned int
    ///  * [ uib]vec[234]
    ///  * mat[234]
    ///  * mat[234]x[234]

    template <typename DataT>
    DataT getUniform(std::string const& name) const
    {
        DataT value;
        auto loc = getUniformLocation(name);
        if (loc >= 0)
            implGetUniform(glTypeOf<DataT>::basetype, loc, (void*)&value);
        else
            value = DataT{};
        return value;
    }

    /// ========================================== UNIFORMS - END ==========================================

private:
    /// Internal generic getter for uniforms
    void implGetUniform(internal::glBaseType type, GLint loc, void* data) const;

public:
    Program();
    ~Program();

    /// Returns true iff program is linked properly
    bool isLinked() const;

    /// Returns a list of all attribute locations
    /// Depending on driver, this only returns "used" attributes
    std::vector<std::pair<std::string, int>> extractAttributeLocations();

    /// Attaches the given shader to this program
    /// Requires re-linking!
    void attachShader(SharedShader const& shader);

    /// Links all attached shader into the program.
    /// Has to be done each time shader and/or IO locations are changed
    /// CAUTION: linking deletes all currently set uniform values!
    void link(bool saveUniformState = true);

    /// Configures this shader for use with transform feedback
    /// NOTE: cannot be done while shader is in use
    void configureTransformFeedback(std::vector<std::string> const& varyings, GLenum bufferMode = GL_INTERLEAVED_ATTRIBS);
    /// Returns true if transform feedback can be used
    bool isConfiguredForTransformFeedback() const { return mTransformFeedbackVaryings.size() > 0; }
    /// Extracts all active uniforms and saves them into a UniformState object
    /// This function should not be used very frequently
    SharedUniformState getUniforms() const;

    /// Binds a uniform buffer to a given block name
    /// DOES NOT REQUIRE PROGRAM USE
    void setUniformBuffer(std::string const& bufferName, SharedUniformBuffer const& buffer);
    /// Binds a shader storage buffer to a given block name
    /// DOES NOT REQUIRE PROGRAM USE
    void setShaderStorageBuffer(std::string const& bufferName, SharedShaderStorageBuffer const& buffer);
    /// Binds an atomic counter buffer to a binding point
    /// DOES NOT REQUIRE PROGRAM USE
    void setAtomicCounterBuffer(int bindingPoint, SharedAtomicCounterBuffer const& buffer);

    /// Verifies registered offsets in the uniform buffer and emits errors if they don't match
    /// Return false if verification failed
    bool verifyUniformBuffer(std::string const& bufferName, SharedUniformBuffer const& buffer);

    /// Activates this shader program.
    /// Deactivation is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED UsedProgram use() { return {this}; }

public: // static construction
    /// Creates a shader program from a list of shaders
    /// Attaches shader and links the program
    /// Program can be used directly after this
    static SharedProgram create(std::vector<SharedShader> const& shader);
    static SharedProgram create(SharedShader const& shader) { return create(std::vector<SharedShader>({shader})); }
    /// Creates a program from either a single file or auto-discovered shader files
    /// E.g. createFromFile("mesh");
    ///   will use mesh.vsh as vertex shader and mesh.fsh as fragment shader if available
    /// See common/shader_endings.cc for a list of available endings
    /// Will also traverse "dots" if file not found to check for base versions
    /// E.g. createFromFile("mesh.opaque");
    ///   might find mesh.opaque.fsh and mesh.vsh
    static SharedProgram createFromFile(std::string const& fileOrBaseName);
    /// Creates a program from a list of explicitly named files
    /// Shader type is determined by common/shader_endings.cc
    static SharedProgram createFromFiles(std::vector<std::string> const& filenames);
};
}
