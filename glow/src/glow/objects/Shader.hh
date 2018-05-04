#pragma once

#include "glow/common/shared.hh"
#include "glow/common/non_copyable.hh"

#include "glow/gl.hh"

#include "NamedObject.hh"

#include <vector>
#include <string>

namespace glow
{
GLOW_SHARED(class, Shader);
GLOW_SHARED(class, ShaderParser);

class Shader final : public NamedObject<Shader, GL_SHADER>
{
    GLOW_NON_COPYABLE(Shader);

    /// Global shader parser
    static SharedShaderParser sParser;

    /// OGL id
    GLuint mObjectName;

    /// Shader type
    GLenum mType;

    /// True iff shader has compile errors
    bool mHasErrors = false;

    /// True iff shader has a compiled version
    bool mCompiled = false;

    /// Filepath of this shader (if applicable)
    std::string mFileName;
    /// Last modification of the file (if applicable)
    int64_t mLastModification = 0;

    /// Filenames with modification times (dependencies)
    std::vector<std::pair<std::string, int64_t>> mFileDependencies;

    /// Primary source of the shader
    std::vector<std::string> mSources;

public: // getter
    GLuint getObjectName() const { return mObjectName; }
    GLenum getType() const { return mType; }
    bool isCompiled() const { return mCompiled; }
    bool hasErrors() const { return mHasErrors; }
    bool isCompiledWithoutErrors() const { return mCompiled && !mHasErrors; }
    /// Returns non-empty filename if created from file, otherwise ""
    std::string const& getFileName() const { return mFileName; }
public:
    Shader(GLenum shaderType);
    ~Shader();

    /// Compiles this shader
    /// Prints to the log should any error occur
    void compile();

    /// Fetches new source from disk (if backed by file)
    /// And recompiles
    void reload();

    /// Sets the shader source to a single string
    void setSource(std::string const& source);
    /// Sets the shader source to a list of strings
    void setSource(std::vector<std::string> const& sources);

    /// Checks the file modification time and returns true if a newer version exists
    /// Returns false for non-file-backed shaders
    bool newerVersionAvailable();

    /// Sets a new shader parser
    /// nullptr is allowed
    static void setShaderParser(SharedShaderParser const& parser);

    /// Returns a non-empty path if the given path points can be
    static bool resolveFile(std::string const& name, GLenum& shaderType, std::string& content, std::string& realFileName);

private:
    /// Adds a file as dependency
    void addDependency(std::string const& filename);

public: // static construction
    /// Creates and compiles (!) a shader from a given source string
    static SharedShader createFromSource(GLenum shaderType, std::string const& source);
    /// Creates and compiles (!) a shader from a given list of source strings
    static SharedShader createFromSource(GLenum shaderType, std::vector<std::string> const& sources);
    /// Creates and compiles (!) a shader by loading the specified file
    static SharedShader createFromFile(GLenum shaderType, std::string const& filename);

    friend class Program;
    friend class ShaderParser;
};
}
