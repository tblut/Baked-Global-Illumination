#pragma once

#include <vector>

#include <glow/common/shared.hh>
#include <glow/common/property.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

#include <glow/fwd.hh>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace glow
{
namespace pipeline
{
struct RenderPass;
GLOW_SHARED(class, RenderingPipeline);
}

namespace debugging
{
/**
 * @brief The DebugRenderer is a easy-to-use (but not so performant) interface for easy debug rendering
 *
 * The DebugRenderer provides a set of functions for drawing primitives:
 *   - drawLine(...)
 *   - drawCube(...)
 *
 * In renderPass(...), call DebugRenderer::renderPass(...)
 *
 * The DebugRenderer accumulates all primitives, unless DebugRenderer::clear() is called
 *
 * Requires:
 *   // setup shader path
 *   DefaultShaderParser::addIncludePath("PATH/TO/glow-extras/debug/shader");
 */
class DebugRenderer
{
private:
    struct PrimitiveVertex
    {
        glm::vec3 pos;

        PrimitiveVertex() = default;
        PrimitiveVertex(float u, float v) : pos(u, v, 0.0) {}
        PrimitiveVertex(glm::vec3 pos) : pos(pos) {}
        PrimitiveVertex(glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoord) : pos(position) {}
        static std::vector<ArrayBufferAttribute> attributes()
        {
            return {
                {&PrimitiveVertex::pos, "aPosition"}, //
            };
        }
    };

    struct Primitive
    {
        SharedVertexArray vao;
        glm::vec4 color;
        glm::mat4 modelMatrix;
    };

    SharedVertexArray mQuad;
    SharedVertexArray mCube;
    SharedVertexArray mLine;
    SharedProgram mShaderOpaque;
    SharedProgram mShaderTransparent;

    std::vector<Primitive> mPrimitives;

public:
    DebugRenderer();

    /// Clears all stored primitives
    void clear();

    /// Renders all stored primitives
    void renderPass(pipeline::RenderPass const& pass) const;

    // render functions
public:
    void renderLine(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0));
    void renderAABB(glm::vec3 start, glm::vec3 end, glm::vec4 color = glm::vec4(1.0), bool wireframe = false);
    void renderAABBCentered(glm::vec3 center, glm::vec3 size, glm::vec4 color = glm::vec4(1.0), bool wireframe = false);
};
GLOW_SHARED(class, DebugRenderer);
}
}
