#pragma once

#include <glm/vec3.hpp>

#include <glow/common/shared.hh>

#include <glow/objects/VertexArray.hh>
#include <glow/objects/ArrayBuffer.hh>

namespace glow
{
GLOW_SHARED(class, VertexArray);

namespace geometry
{
/// Builder for a line (GL_LINES) with 2 vertices
/// VertexT needs a c'tor that can be called with (x,y,z)
/// or supply a VertexCreator object which can be called as such
/// e.g. VertexT make_vertex(glm::vec3 p)
///
/// Supported built-in types:
///   vec3
///
/// If you use your custom vertex struct you have 2 options:
///   A: specify all attributes in Line(...) ctor
///   B: implemented the following function in your VertexT:
///      static std::vector<ArrayBufferAttribute> attributes();
///
/// Usage:
///   * auto lineVA = geometry::Line<>().generate();
///   * auto lineVA = geometry::Line<glm::vec3>().generate();
///   * auto lineVA = geometry::Line<MyVertex>().generate();
///     with struct MyVertex { ...
///         static std::vector<ArrayBufferAttribute> attributes() { return { {&MyVertex::a, "a"}, ... }; }
///     };
///   * auto lineVA = geometry::Line<MyVertex>({ {&MyVertex::a, "a"}, ... }).generate();
///     with struct MyVertex { ... };
template <typename VertexT = glm::vec3>
struct Line
{
public:
    /// List of line attributes
    std::vector<ArrayBufferAttribute> attributes;

    /// start coordinate
    glm::vec3 startPos;
    /// end coordinate
    glm::vec3 endPos;

public: // Predefined creators
    static VertexT createVertex(glm::vec3 p) { return VertexT{p}; }
public:
    Line(std::vector<ArrayBufferAttribute> const& attrs = attributesOf((VertexT*)0),
         glm::vec3 start = {0, 0, 0},
         glm::vec3 end = {1, 0, 0})
      : attributes(attrs), startPos(start), endPos(end)
    {
    }
    /**
     * @brief generates a Vertex Array with a single Array Buffer containing line data
     *
     * Vertex layout:
     *   VertexT{0, 0}
     *   VertexT{0, 1}
     *   VertexT{1, 0}
     *   VertexT{1, 1}
     */
    template <typename VertexCreator = decltype(createVertex)>
    SharedVertexArray generate(VertexCreator&& gen = createVertex) const
    {
        auto ab = ArrayBuffer::create();
        ab->setObjectLabel("Line");
        ab->defineAttributes(attributes);
        VertexT data[] = {
            gen(startPos), //
            gen(endPos),   //
        };
        ab->bind().setData(data);
        auto va = VertexArray::create(ab, nullptr, GL_LINES);
        ab->setObjectLabel("Line");
        return va;
    }

public: // Predefined attributes
    static std::vector<ArrayBufferAttribute> attributesOf(glm::vec3*)
    {
        return {ArrayBufferAttribute("aPosition", GL_FLOAT, 3, 0)};
    }
    static std::vector<ArrayBufferAttribute> attributesOf(void*) { return VertexT::attributes(); }
};
}
}
