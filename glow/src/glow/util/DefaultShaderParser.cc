#include "DefaultShaderParser.hh"

#include <set>
#include <algorithm>
#include <sstream>
#include <fstream>

#include <glow/glow.hh>
#include <glow/common/str_utils.hh>
#include <glow/common/shader_endings.hh>
#include <glow/common/stream_readall.hh>
#include <glow/common/log.hh>

#include <glow/objects/Shader.hh>

using namespace glow;

std::vector<std::string> DefaultShaderParser::sIncludePaths = {"."};


void DefaultShaderParser::parseWithInclude(Shader *shader,
                                           std::stringstream &parsedSrc,
                                           const std::string &source,
                                           int sourceIdx,
                                           int &nextSrcIdx,
                                           std::set<std::string> &includes,
                                           const std::string &relativePath)
{
    auto src = source;

    // replace \r by \n for consistent line endings
    replace(begin(src), end(src), '\r', '\n');

    // add __LINE__ and __FILE__ info to src
    parsedSrc << "#line 1 " << sourceIdx << "\n";

    std::stringstream ss(src);
    std::string line;
    auto lineIdx = 0u;
    while (getline(ss, line, '\n'))
    {
        ++lineIdx;

        // skip #version tags (and empty strings)
        if (line.find("#version ") == line.find_first_not_of(" \t"))
        {
            parsedSrc << "\n";
            continue;
        }

        // resolve includes
        if (line.find("#include ") == line.find_first_not_of(" \t"))
        {
            // perform include

            // absolute
            if (line.find('<') != std::string::npos)
            {
                auto p0 = line.find('<');
                auto p1 = line.find('>');

                if (p0 > p1 || p1 == std::string::npos)
                    error() << "Include `" << line << "' not recognized/invalid.";
                else
                {
                    auto file = line.substr(p0 + 1, p1 - p0 - 1);
                    auto absfile = resolve(file, "");

                    if (absfile.empty())
                        error() << "Could not resolve `" << line << "'.";
                    else
                    {
                        if (includes.insert(absfile).second) // pragma once
                        {
                            addDependency(shader, absfile);
                            std::ifstream fs(absfile);
                            auto incIdx = nextSrcIdx;
                            ++nextSrcIdx;
                            parseWithInclude(shader, parsedSrc, util::readall(fs), incIdx, nextSrcIdx, includes,
                                             util::pathOf(absfile));
                        }
                    }
                }
            }
            // relative
            else if (line.find('"'))
            {
                auto p0 = line.find('"');
                auto p1 = line.rfind('"');

                if (p0 > p1 || p1 == std::string::npos)
                    error() << "Include `" << line << "' not recognized/invalid.";
                else
                {
                    auto file = line.substr(p0 + 1, p1 - p0 - 1);
                    auto absfile = resolve(file, relativePath);

                    if (absfile.empty())
                        error() << "Could not resolve `" << line << "'.";
                    else
                    {
                        if (includes.insert(absfile).second) // pragma once
                        {
                            addDependency(shader, absfile);
                            std::ifstream fs(absfile);
                            auto incIdx = nextSrcIdx;
                            ++nextSrcIdx;
                            parseWithInclude(shader, parsedSrc, util::readall(fs), incIdx, nextSrcIdx, includes,
                                             util::pathOf(absfile));
                        }
                    }
                }
            }
            else
                error() << "Include `" << line << "' not recognized/invalid.";

            // resume file
            parsedSrc << "#line " << lineIdx << " " << sourceIdx << "\n";
            parsedSrc << "\n";
            continue;
        }

        parsedSrc << line << "\n";
    }
}

std::string DefaultShaderParser::resolve(const std::string &filename, const std::string &relPath)
{
    if (filename.empty())
    {
        error() << "Empty include path";
        return "";
    }

    if (filename[0] == '/' || filename[0] == '\\')
    {
        error() << "Filename `" << filename << "' is absolute. Not supported.";
        return "";
    }

    // check rel path
    if (!relPath.empty() && std::ifstream(relPath + "/" + filename).good())
        return relPath + "/" + filename;

    // check inc paths
    for (auto const &path : sIncludePaths)
        if (std::ifstream(path + "/" + filename).good())
            return path + "/" + filename;

    return "";
}

DefaultShaderParser::DefaultShaderParser()
{
}

void DefaultShaderParser::setIncludePaths(const std::vector<std::string> &paths)
{
    sIncludePaths = paths;
}

void DefaultShaderParser::addIncludePath(const std::string &path)
{
    for (auto const& p : sIncludePaths)
        if (p == path)
            return;

    sIncludePaths.push_back(path);
}

std::vector<std::string> DefaultShaderParser::parse(Shader *shader, const std::vector<std::string> &sources)
{
    // may be empty
    auto sFilename = shader->getFileName();
    auto relPath = util::pathOf(sFilename);
    if (!sFilename.empty() && relPath.empty())
        relPath = ".";

    std::set<std::string> includes;

    std::stringstream parsedSrc;
#ifdef GLOW_OPENGL_PROFILE_CORE
    parsedSrc << "#version " + std::to_string(glow::OGLVersion.total * 10) + " core\n";
#else
    parsedSrc << "#version " + std::to_string(glow::OGLVersion.total * 10) + "\n";
#endif

    int nextSrcIdx = sources.size();

    for (auto sIdx = 0u; sIdx < sources.size(); ++sIdx)
        parseWithInclude(shader, parsedSrc, sources[sIdx], sIdx, nextSrcIdx, includes, relPath);

    return {parsedSrc.str()};
}

bool DefaultShaderParser::resolveFile(const std::string &name, GLenum &shaderType, std::string &content, std::string &realFileName)
{
    if (name.empty())
        return false;

    // detect shader type
    auto found = false;
    for (auto const &kvp : glow::shaderEndingToType)
        if (util::endswith(name, kvp.first))
        {
            found = true;
            shaderType = kvp.second;
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

    // includes if not absolute
    if (name[0] != '/')
    {
        for (auto const &inc : sIncludePaths)
        {
            if (std::ifstream(inc + "/" + name).good())
            {
                if (!found) // has to be done here, because non-existant files should fail silently
                {
                    error() << "Could not deduce shader type of Shader file " << name << ".";
                    return false;
                }

                realFileName = inc + "/" + name;
                std::ifstream fs(realFileName);
                content = util::readall(fs);
                return true;
            }
        }
    }

    return false;
}
