#include "DebugRenderer.hh"

#include <glm/ext.hpp>

#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/Line.hh>

#include <glow-extras/pipeline/RenderPass.hh>

#include <glow-extras/camera/FixedCamera.hh>

#include <glow/objects/Program.hh>

using namespace glow;
using namespace glow::debugging;
using namespace glow::pipeline;

DebugRenderer::DebugRenderer()
{
    mQuad = geometry::Quad<PrimitiveVertex>().generate();
    mCube = geometry::Cube<PrimitiveVertex>().generate();
    mLine = geometry::Line<PrimitiveVertex>().generate();

    mShaderOpaque = Program::createFromFiles({"glow-debugging/primitive.opaque.fsh", "glow-debugging/primitive.vsh"});
    mShaderTransparent
        = Program::createFromFiles({"glow-debugging/primitive.transparent.fsh", "glow-debugging/primitive.vsh"});
}

void DebugRenderer::clear()
{
    mPrimitives.clear();
}

void DebugRenderer::renderPass(const pipeline::RenderPass& pass) const
{
    switch (pass.type)
    {
    case RenderPassType::Opaque:
    {
        auto shader = mShaderOpaque->use();
        shader.setUniform("uView", pass.camera->getViewMatrix());
        shader.setUniform("uProj", pass.camera->getProjectionMatrix());

        for (auto const& p : mPrimitives)
            if (p.color.a >= 1.0)
            {
                shader.setUniform("uColor", p.color);
                shader.setUniform("uModel", p.modelMatrix);

                p.vao->bind().draw();
            }
    }
    break;
    case RenderPassType::Transparent:
    {
        auto shader = mShaderTransparent->use();
        shader.setUniform("uView", pass.camera->getViewMatrix());
        shader.setUniform("uProj", pass.camera->getProjectionMatrix());

        for (auto const& p : mPrimitives)
            if (p.color.a < 1.0)
            {
                shader.setUniform("uColor", p.color);
                shader.setUniform("uModel", p.modelMatrix);

                p.vao->bind().draw();
            }
    }
    break;
    default:
        break;
    }
}

void DebugRenderer::renderLine(glm::vec3 start, glm::vec3 end, glm::vec4 color)
{
    auto m = translate(start) * glm::mat4(glm::mat3(end - start, glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
    mPrimitives.push_back({mLine, color, m});
}

void DebugRenderer::renderAABB(glm::vec3 start, glm::vec3 end, glm::vec4 color, bool wireframe)
{
    if (start.x > end.x)
        std::swap(start.x, end.x);
    if (start.y > end.y)
        std::swap(start.y, end.y);
    if (start.z > end.z)
        std::swap(start.z, end.z);

    if (!wireframe)
    {
        auto m = translate((start + end) / 2.0f) * scale((end - start) / 2.0f);
        mPrimitives.push_back({mCube, color, m});
    }
    else
    {
        auto s = start;
        auto e = end;

        renderLine({s.x, s.y, s.z}, {e.x, s.y, s.z}, color);
        renderLine({s.x, s.y, e.z}, {e.x, s.y, e.z}, color);
        renderLine({s.x, e.y, s.z}, {e.x, e.y, s.z}, color);
        renderLine({s.x, e.y, e.z}, {e.x, e.y, e.z}, color);

        renderLine({s.x, s.y, s.z}, {s.x, e.y, s.z}, color);
        renderLine({s.x, s.y, e.z}, {s.x, e.y, e.z}, color);
        renderLine({e.x, s.y, s.z}, {e.x, e.y, s.z}, color);
        renderLine({e.x, s.y, e.z}, {e.x, e.y, e.z}, color);

        renderLine({s.x, s.y, s.z}, {s.x, s.y, e.z}, color);
        renderLine({e.x, s.y, s.z}, {e.x, s.y, e.z}, color);
        renderLine({s.x, e.y, s.z}, {s.x, e.y, e.z}, color);
        renderLine({e.x, e.y, s.z}, {e.x, e.y, e.z}, color);
    }
}

void DebugRenderer::renderAABBCentered(glm::vec3 center, glm::vec3 size, glm::vec4 color, bool wireframe)
{
    renderAABB(center - size / 2.0f, center + size / 2.0f, color, wireframe);
}
