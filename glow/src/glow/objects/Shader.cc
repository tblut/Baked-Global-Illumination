#include "Shader.hh"

#include <fstream>

#include "glow/glow.hh"
#include "glow/util/DefaultShaderParser.hh"

#include "glow/common/file_utils.hh"
#include "glow/common/log.hh"
#include "glow/common/stream_readall.hh"
#include "glow/common/shader_endings.hh"
#include "glow/common/str_utils.hh"
#include "glow/common/profiling.hh"

using namespace glow;

SharedShaderParser Shader::sParser = std::make_shared<DefaultShaderParser>();

Shader::Shader(GLenum shaderType) : mType(shaderType)
{
    checkValidGLOW();
    mObjectName = glCreateShader(mType);
}

Shader::~Shader()
{
    checkValidGLOW();
    glDeleteShader(mObjectName);
}

void Shader::compile()
{
    checkValidGLOW();
    GLOW_ACTION();
    glCompileShader(mObjectName);

    // check error log
    GLint logLength = 0;
    glGetShaderiv(mObjectName, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        std::vector<GLchar> log;
        log.resize(logLength + 1);
        glGetShaderInfoLog(mObjectName, logLength + 1, nullptr, log.data());

        error() << "Log for " << (mFileName.empty() ? to_string(this) : mFileName);
        error() << "Shader compiler: " << log.data();
        mHasErrors = true;
        mCompiled = false; // is this true?
    }
    else
    {
        mHasErrors = false;
        mCompiled = true;
    }
}

void Shader::reload()
{
    GLOW_ACTION();

    // reload source
    if (!mFileName.empty())
    {
        std::ifstream file(mFileName);
        if (!file.good())
        {
            warning() << "Skipping reload for " << mFileName << ", file not readable. " << to_string(this);
            return;
        }
        setSource(util::readall(file));
        mLastModification = util::fileModificationTime(mFileName);
    }

    // compile
    compile();
}

void Shader::setSource(const std::string &source)
{
    setSource(std::vector<std::string>({source}));
}

void Shader::setSource(const std::vector<std::string> &sources)
{
    checkValidGLOW();
    mFileDependencies.clear();
    mSources = sources;

    auto parsedSources = sParser ? sParser->parse(this, sources) : sources;

    std::vector<const GLchar *> srcs;
    srcs.resize(parsedSources.size());
    for (auto i = 0u; i < parsedSources.size(); ++i)
        srcs[i] = parsedSources[i].c_str();
    glShaderSource(mObjectName, srcs.size(), srcs.data(), nullptr);
}

SharedShader Shader::createFromSource(GLenum shaderType, const std::string &source)
{
    GLOW_ACTION();

    auto shader = std::make_shared<Shader>(shaderType);
    shader->setSource(source);
    shader->compile();
    return shader;
}

SharedShader Shader::createFromSource(GLenum shaderType, const std::vector<std::string> &sources)
{
    GLOW_ACTION();

    auto shader = std::make_shared<Shader>(shaderType);
    shader->setSource(sources);
    shader->compile();
    return shader;
}

SharedShader Shader::createFromFile(GLenum shaderType, const std::string &filename)
{
    GLOW_ACTION();

    std::ifstream shaderFile(filename);
    if (!shaderFile.good())
    {
        error() << "Unable to read shader file " << filename;
        return nullptr;
    }

    auto shader = std::make_shared<Shader>(shaderType);
    shader->setObjectLabel(filename);
    shader->mFileName = filename;
    shader->mLastModification = util::fileModificationTime(filename);
    shader->setSource(util::readall(shaderFile));
    shader->compile();
    return shader;
}

bool Shader::newerVersionAvailable()
{
    // check if dependency changed
    for (auto const &kvp : mFileDependencies)
        if (util::fileModificationTime(kvp.first) > kvp.second)
            return true;

    // see if file-backed
    if (!mFileName.empty() && std::ifstream(mFileName).good())
    {
        if (util::fileModificationTime(mFileName) > mLastModification)
            return true;
    }

    return false;
}

void Shader::setShaderParser(const SharedShaderParser &parser)
{
    sParser = parser;
}

bool Shader::resolveFile(const std::string &name, GLenum &shaderType, std::string &content, std::string &realFileName)
{
    if (sParser)
        return sParser->resolveFile(name, shaderType, content, realFileName);

    // no parser: fallback

    // detect shader type
    auto found = false;
    for (auto const &kvp : glow::shaderEndingToType)
        if (util::endswith(name, kvp.first))
        {
            shaderType = kvp.second;
            found = true;
            break;
        }

    // direct match
    if (std::ifstream(name).good())
    {
        if (!found) // has to be done here, because non-existant files should fail silently
        {
            error() << "Could not deduce shader type of Shader file " << name << ".";
            return false;
        }

        realFileName = name;
        std::ifstream fs(realFileName);
        content = util::readall(fs);
        return true;
    }

    return false;
}

void Shader::addDependency(const std::string &filename)
{
    // ignore multi-adds
    for (auto const &kvp : mFileDependencies)
        if (kvp.first == filename)
            return;

    mFileDependencies.push_back(make_pair(filename, util::fileModificationTime(filename)));
}
