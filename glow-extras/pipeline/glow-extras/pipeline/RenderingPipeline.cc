#include "RenderingPipeline.hh"

#include <glow/common/scoped_gl.hh>

#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/TextureRectangle.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/camera/GenericCamera.hh>

#include <glow-extras/geometry/Quad.hh>

#include <glm/ext.hpp>

using namespace glow;
using namespace glow::pipeline;

int RenderingPipeline::getOutputWidth() const
{
    return mCamera ? mCamera->getViewportWidth() : mWidth;
}

int RenderingPipeline::getOutputHeight() const
{
    return mCamera ? mCamera->getViewportHeight() : mHeight;
}

RenderingPipeline::RenderingPipeline()
{
    mClearColor = glm::pow(glm::vec3(0 / 255.f, 84 / 255.f, 159 / 255.f), glm::vec3(2.2f));

    mFullSizeTargets.push_back(mTargetNormals = TextureRectangle::create(mWidth, mHeight, GL_RGB16F));
    mFullSizeTargets.push_back(mTargetColor = TextureRectangle::create(mWidth, mHeight, GL_RGB16F));
    mFullSizeTargets.push_back(mTargetColorTmp = TextureRectangle::create(mWidth, mHeight, GL_RGB16F));
    mFullSizeTargets.push_back(mTargetTranspAccum = TextureRectangle::create(mWidth, mHeight, GL_RGBA16F));
    mFullSizeTargets.push_back(mTargetTranspReveal = TextureRectangle::create(mWidth, mHeight, GL_R16F));
    mFullSizeTargets.push_back(mTargetSSAO = TextureRectangle::create(mWidth, mHeight, GL_R16F));
    mFullSizeTargets.push_back(mTargetDepth = TextureRectangle::create(mWidth, mHeight, GL_DEPTH_COMPONENT32));
    mFullSizeTargets.push_back(mTargetDepthPre = TextureRectangle::create(mWidth, mHeight, GL_DEPTH_COMPONENT32));

    mShaderOutput = Program::createFromFiles({"glow-pipeline/output.fsh", "glow-pipeline/fullscreen.vsh"});
    mShaderFXAA = Program::createFromFiles({"glow-pipeline/fxaa.fsh", "glow-pipeline/fullscreen.vsh"});
    mShaderToneMap = Program::createFromFiles({"glow-pipeline/tonemap.fsh", "glow-pipeline/fullscreen.vsh"});
    mShaderAlphaResolve = Program::createFromFiles({"glow-pipeline/alpha-resolve.fsh", "glow-pipeline/fullscreen.vsh"});

    mVaoQuad = geometry::Quad<>().generate();

    mFboZPre = Framebuffer::create({{"fNormal", mTargetNormals}}, mTargetDepth);
    mFboForward = Framebuffer::create({{"fColor", mTargetColor}}, mTargetDepth);
    mFboTransparent = Framebuffer::create({{"fAccum", mTargetTranspAccum}, {"fRevealage", mTargetTranspReveal}}, mTargetDepth);
    mFboToColor = Framebuffer::create({{"fColor", mTargetColor}}, mTargetDepth);
    mFboToColorTmp = Framebuffer::create({{"fColor", mTargetColorTmp}}, mTargetDepth);
}

void RenderingPipeline::resize(int w, int h)
{
    if (w == mWidth && h == mHeight)
        return; // no change

    mWidth = w;
    mHeight = h;

    for (auto const& target : mFullSizeTargets)
        target->bind().resize(w, h);
}

void RenderingPipeline::assignCamera(const camera::SharedGenericCamera& cam, bool useCamViewport)
{
    mCamera = cam;

    if (cam && useCamViewport)
        resize(cam->getViewportWidth(), cam->getViewportHeight());
}

void RenderingPipeline::render(const std::function<void(RenderPass const& pass)>& renderFunc)
{
    assert(renderFunc != nullptr && "no render function provided");
    assert(mCamera != nullptr && "no camera provided");

    // Shadows are updated separately

    // TODO: inverse z-Buffer

    camera::FixedCamera cam(mCamera->getPosition(),         //
                            mCamera->getViewMatrix(),       //
                            mCamera->getProjectionMatrix(), //
                            mCamera->getViewportSize());

    {
        GLOW_SCOPED(enable, GL_DEPTH_TEST);
        GLOW_SCOPED(enable, GL_CULL_FACE);
        // GLOW_SCOPED(viewport, 0, 0, mWidth, mHeight); // - automatic

        // z-Pre
        {
            RenderPass rp;
            rp.type = RenderPassType::ZPre;
            rp.pipeline = this;
            rp.camera = &cam;
            rp.depthTarget = mTargetDepth;
            rp.framebuffer = mFboZPre;

            auto fbo = mFboZPre->bind();

            GLOW_SCOPED(clearColor, 0, 0, 0, 1); // zero normal
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderFunc(rp);

            // TODO: copy depth to mTargetDepthPre
        }

        // TODO: SSAO

        // TODO: Light prePass

        // Forward
        {
            RenderPass rp;
            rp.type = RenderPassType::Opaque;
            rp.pipeline = this;
            rp.camera = &cam;
            rp.depthTarget = mTargetDepth;
            rp.framebuffer = mFboForward;

            auto fbo = mFboForward->bind();

            GLOW_SCOPED(clearColor, mClearColor.x, mClearColor.y, mClearColor.z, 1);
            glClear(GL_COLOR_BUFFER_BIT); // keep depth

            renderFunc(rp);
        }

        // Transparency
        if (mTransparentPass)
        {
            RenderPass rp;
            rp.type = RenderPassType::Transparent;
            rp.pipeline = this;
            rp.camera = &cam;
            rp.depthTarget = mTargetDepth;
            rp.framebuffer = mFboTransparent;

            auto fbo = mFboTransparent->bind();

            // clear accum to vec4(0), revealage to float(1)
            glm::vec4 clearAccum(0, 0, 0, 0);
            float clearRevealage(1);
            glClearBufferfv(GL_COLOR, 0, glm::value_ptr(clearAccum));
            glClearBufferfv(GL_COLOR, 1, &clearRevealage);

            // disable depth-write
            GLOW_SCOPED(depthMask, GL_FALSE);

            // configure blending
            GLOW_SCOPED(enable, GL_BLEND);
            glBlendFunci(0, GL_ONE, GL_ONE);                  // accum
            glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); // multiplicative

            // culling is default off
            GLOW_SCOPED(disable, GL_CULL_FACE);

            renderFunc(rp);
        }
    }

    {
        GLOW_SCOPED(depthMask, GL_FALSE);
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        auto toColorTmp = true;

        // Post-processing
        {
            // GLOW_SCOPED(viewport, 0, 0, mWidth, mHeight); // - automatic

            // Postprocess pass after opaque
            {
                RenderPass rp;
                rp.type = RenderPassType::PostprocessOpaque;
                rp.pipeline = this;
                rp.camera = &cam;
                rp.depthTarget = mTargetDepth;
                rp.framebuffer = (toColorTmp ? mFboToColor : mFboToColorTmp);

                auto fbo = (toColorTmp ? mFboToColor : mFboToColorTmp)->bind();

                renderFunc(rp);
            }

            // Transparency resolve
            if (mTransparentPass)
            {
                auto fbo = (toColorTmp ? mFboToColor : mFboToColorTmp)->bind();

                GLOW_SCOPED(enable, GL_BLEND);
                GLOW_SCOPED(blendFunc, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

                auto shader = mShaderAlphaResolve->use();
                shader.setTexture("uTexAccum", mTargetTranspAccum);
                shader.setTexture("uTexRevealage", mTargetTranspReveal);

                mVaoQuad->bind().draw();

                // NO invert because blending
            }

            // Postprocess pass after transparency
            {
                RenderPass rp;
                rp.type = RenderPassType::PostprocessAll;
                rp.pipeline = this;
                rp.camera = &cam;
                rp.depthTarget = mTargetDepth;
                rp.framebuffer = (toColorTmp ? mFboToColor : mFboToColorTmp);

                auto fbo = (toColorTmp ? mFboToColor : mFboToColorTmp)->bind();

                renderFunc(rp);
            }

            // TODO: ToneMapping, Gamma Correction
            {
                auto fbo = (toColorTmp ? mFboToColorTmp : mFboToColor)->bind();

                auto shader = mShaderToneMap->use();
                shader.setTexture("uTexture", toColorTmp ? mTargetColor : mTargetColorTmp);

                mVaoQuad->bind().draw();

                toColorTmp = !toColorTmp;
            }

            // FXAA
            if (mFXAA)
            {
                auto fbo = (toColorTmp ? mFboToColorTmp : mFboToColor)->bind();

                auto shader = mShaderFXAA->use();
                shader.setTexture("uTexture", toColorTmp ? mTargetColor : mTargetColorTmp);

                mVaoQuad->bind().draw();

                toColorTmp = !toColorTmp;
            }
        }

        // Output
        {
            GLOW_SCOPED(viewport, 0, 0, getOutputWidth(), getOutputHeight());

            if (mWidth / (float)getOutputWidth() > 2.01 || mHeight / (float)getOutputHeight() > 2.01)
            {
                static bool warnOnce = false;
                if (!warnOnce)
                {
                    warning() << "Upscaling of more than factor 2 is not supported yet and will result in worse "
                                 "quality";
                    warnOnce = true;
                }
            }

            // copy shader (with downsampling and dithering)
            auto shader = mShaderOutput->use();
            shader.setTexture("uTexture", toColorTmp ? mTargetColor : mTargetColorTmp);
            shader.setUniform("uOutputSize", glm::vec2(getOutputWidth(), getOutputHeight()));
            shader.setUniform("uDitheringStrength", mDitheringStrength);

            mVaoQuad->bind().draw();
        }
    }
}

bool RenderingPipeline::queryPosition3D(int x, int y, glm::vec3* pos, float* depth)
{
    if (!mCamera)
        return false;
    if (x < 0 || y < 0 || x >= (int)mCamera->getViewportWidth() || y >= (int)mCamera->getViewportHeight())
        return false;

    // rescale position
    x = x * mWidth / mCamera->getViewportWidth();
    y = mHeight - y * mHeight / mCamera->getViewportHeight() - 1;

    auto fb = mFboForward->bind();
    float d;

    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);
    if (depth)
        *depth = d;

    if (d < 1)
    {
        // unproject (with viewport coords!)
        glm::vec4 v{x / float(mWidth) * 2 - 1, y / float(mHeight) * 2 - 1, d * 2 - 1, 1.0};

        v = glm::inverse(mCamera->getProjectionMatrix()) * v;
        v /= v.w;
        v = glm::inverse(mCamera->getViewMatrix()) * v;
        if (pos)
            *pos = glm::vec3(v);

        return true;
    }

    return false;
}

SharedRenderingPipeline RenderingPipeline::create(const camera::SharedGenericCamera& cam)
{
    auto pipe = std::make_shared<RenderingPipeline>();
    pipe->assignCamera(cam);
    return pipe;
}
