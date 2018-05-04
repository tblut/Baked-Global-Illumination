#pragma once

#include <glm/vec2.hpp>

#include <glow/common/shared.hh>

#include <glow/objects/VertexArray.hh>
#include <glow/objects/ArrayBuffer.hh>

namespace glow
{
GLOW_SHARED(class, VertexArray);

namespace geometry
{
/// Builder for a quad (GL_TRIANGLE_STRIP) with 4 vertices
/// VertexT needs a c'tor that can be called with (0, 0) to (1, 1)
/// or supply a VertexCreator object which can be called as such
/// e.g. VertexT make_vertex(float u, float v)
///
/// Supported built-in types:
///   vec2
///
/// If you use your custom vertex struct you have 2 options:
///   A: specify all attributes in Quad(...) ctor
///   B: implemented the following function in your VertexT:
///      static std::vector<ArrayBufferAttribute> attributes();
///
/// Usage:
///   * auto quadVA = geometry::Quad<>().generate();
///   * auto quadVA = geometry::Quad<glm::vec2>().generate();
///   * auto quadVA = geometry::Quad<MyVertex>().generate();
///     with struct MyVertex { ...
///         static std::vector<ArrayBufferAttribute> attributes() { return { {&MyVertex::a, "a"}, ... }; }
///     };
///   * auto quadVA = geometry::Quad<MyVertex>({ {&MyVertex::a, "a"}, ... }).generate();
///     with struct MyVertex { ... };
template <typename VertexT = glm::vec2>
struct Quad
{
public:
    /// List of quad attributes
    std::vector<ArrayBufferAttribute> attributes;

    /// Minimal coordinate
    glm::vec2 minCoord;
    /// Maximal coordinate
    glm::vec2 maxCoord;

public: // Predefined creators
    static VertexT createVertex(float u, float v) { return VertexT{u, v}; }
public:
    Quad(std::vector<ArrayBufferAttribute> const& attrs = attributesOf((VertexT*)0),
         glm::vec2 minCoord = {0, 0},
         glm::vec2 maxCoord = {1, 1})
      : attributes(attrs), minCoord(minCoord), maxCoord(maxCoord)
    {
    }
    /**
     * @brief generates a Vertex Array with a single Array Buffer containing quad data
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
        ab->setObjectLabel("Quad");
        ab->defineAttributes(attributes);
        VertexT data[] = {
            gen(minCoord.x, minCoord.y), //
            gen(minCoord.x, maxCoord.y), //
            gen(maxCoord.x, minCoord.y), //
            gen(maxCoord.x, maxCoord.y),
        };
        ab->bind().setData(data);
        auto va = VertexArray::create(ab, nullptr, GL_TRIANGLE_STRIP);
        va->setObjectLabel("Quad");
        return va;
    }

public: // Predefined attributes
    static std::vector<ArrayBufferAttribute> attributesOf(glm::vec2*)
    {
        return {ArrayBufferAttribute("aPosition", GL_FLOAT, 2, 0)};
    }
    static std::vector<ArrayBufferAttribute> attributesOf(void*) { return VertexT::attributes(); }
};
}
}
