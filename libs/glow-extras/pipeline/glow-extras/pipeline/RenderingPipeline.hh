#pragma once

#include <vector>
#include <functional>

#include "RenderPass.hh"

#include <glow/fwd.hh>
#include <glow/common/shared.hh>
#include <glow/common/property.hh>

#include <glm/vec3.hpp>

namespace glow
{
namespace camera
{
GLOW_SHARED(class, FixedCamera);
GLOW_SHARED(class, GenericCamera);
}

namespace pipeline
{
GLOW_SHARED(class, RenderingPipeline);
/**
 * A Forward+ Rendering Pipeline with post-processing support
 *
 * Features:
 *  * Tiled Light Rendering
 *  * Z-PrePass
 *  * Soft Shadows
 *  * Inverse Depth Buffer
 *  * Dithering
 *  * HDR
 *
 * Basic usage:
 *   // setup shader path
 *   DefaultShaderParser::addIncludePath("PATH/TO/glow-extras/pipeline/shader");
 *
 *   // creation
 *   pipeline = RenderingPipeline::create(myCamera);
 *
 *   // configuration
 *   pipeline->setXYZ(...); // optional
 *
 *   // rendering
 *   pipeline->updateShadows(...); // optional
 *   pipeline->render(myRenderFunc);
 *
 * Transparency:
 *   This pipeline implements Weighted Blended Order-Independent Transparency (http://jcgt.org/published/0002/02/09/)
 *   Use #include <glow-pipeline/transparency.glsl> (that file includes further documentation)
 *
 */
class RenderingPipeline
{
private:
    /// Generic Camera used for rendering
    camera::SharedGenericCamera mCamera = nullptr;

    /// Render width in px
    int mWidth = 2;
    /// Render height in px
    int mHeight = 2;

    // === RENDER TARGETS ===
    SharedTextureRectangle mTargetNormals;
    SharedTextureRectangle mTargetColor;
    SharedTextureRectangle mTargetColorTmp;
    SharedTextureRectangle mTargetTranspAccum;
    SharedTextureRectangle mTargetTranspReveal;
    SharedTextureRectangle mTargetDepth;
    SharedTextureRectangle mTargetDepthPre;
    SharedTextureRectangle mTargetSSAO;
    std::vector<SharedTextureRectangle> mFullSizeTargets;

    // === Framebuffers ===
    SharedFramebuffer mFboZPre;
    SharedFramebuffer mFboForward;
    SharedFramebuffer mFboToColor;
    SharedFramebuffer mFboToColorTmp;
    SharedFramebuffer mFboTransparent;

    // === Shader Programs ===
    SharedProgram mShaderOutput;
    SharedProgram mShaderFXAA;
    SharedProgram mShaderToneMap;
    SharedProgram mShaderAlphaResolve;

    // === VAOs ===
    SharedVertexArray mVaoQuad;

    // === SETTINGS ===
    bool mFXAA = true;
    bool mTransparentPass = true;
    float mDitheringStrength = 1 / 256.f;
    glm::vec3 mClearColor;

public: // getter, setter
    GLOW_PROPERTY(Camera);

    GLOW_GETTER(Width);
    GLOW_GETTER(Height);

    GLOW_PROPERTY(FXAA);
    GLOW_PROPERTY(DitheringStrength);
    GLOW_PROPERTY(TransparentPass);
    GLOW_PROPERTY(ClearColor);

    int getOutputWidth() const;
    int getOutputHeight() const;

public:
    RenderingPipeline();

    /// Resizes the internal pipeline size
    void resize(int w, int h);

    /// Updates the camera
    /// If useCamViewport is true, also resizes the pipeline
    void assignCamera(camera::SharedGenericCamera const& cam, bool useCamViewport = true);

    /**
     * @brief executes the whole rendering pipeline
     * @param renderFunc function to call for every renderpass. Use the provided pass information
     */
    void render(std::function<void(RenderPass const& pass)> const& renderFunc);

public:
    /// Reads back part of the depth buffer and calculates the 3d position of the pixel
    /// returns false if depth is background (== 1)
    /// (x,y) are in camera pixel coords (they'll get scaled if internal size differs)
    /// (0,0) is top-left
    bool queryPosition3D(int x, int y, glm::vec3* pos, float* depth = nullptr);

public: // static creation
    /// Creates a new rendering pipeline and optionally assigns a camera to it
    static SharedRenderingPipeline create(camera::SharedGenericCamera const& cam = nullptr);
};
}
}
