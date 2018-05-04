#include "FixedCamera.hh"

#include <glm/glm.hpp>

using namespace glow::camera;

FixedCamera::FixedCamera()
{
}

FixedCamera::FixedCamera(const glm::vec3 &_pos, const glm::mat4 &_view, const glm::mat4 &_proj, const glm::uvec2 &_viewport)
  : mPosition(_pos), mViewMatrix(_view), mViewportSize(_viewport)
{
    setProjectionMatrix(_proj);
}

void FixedCamera::setProjectionMatrix(glm::mat4 const &_val)
{
    mProjectionMatrix = _val;

    //
    // Calculate near and far plane from the matrix
    // As the matrix does not have to be a standard OpenGL
    // projection matrix, we can't derrive the values from
    // the matrix entries directly but we have to reproject
    // points from the NDC cube... ...but as some matrices
    // project to -1..1 and others to 0..1 we have to figure
    // this out first... ...keeping in mind, that some also
    // invert the range inside of the Z buffer, so Z=1 might
    // be the near plane while Z=0 is the far plane...
    // ...oh, and the far plane might be at infinity...
    //

    //
    // First step, figure out which values project to
    // -1, 0 and 1:
    //
    glm::mat4 invProj = glm::inverse(mProjectionMatrix);
    glm::vec4 tmp = invProj * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    float reproj_minusOne = tmp.z / tmp.w;
    tmp = invProj * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float reproj_Zero = tmp.z / tmp.w;
    tmp = invProj * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    float reproj_One = tmp.z / tmp.w;

    //
    // Next step, figure out which value is the near plane and which is the far plane.
    //
    // right handed coordinate system -> the camera was looking along the negative Z axis
    // if it was an OpenGL matrix, the projections will result in -near, -something, -far
    // if it was an OpenGL matrix with infinit far plane, the projections will result in -near, -something, -inf
    // if it was a reverse DX style matrix it will result in +near, -inf, -near

    if (reproj_minusOne > 0.0f)
    {
        // near,far mapped to 1,0 or 0,1 as -1 was mapped to a positive value which is behind the camera
        // if we assume we are looking along the negative Z axis!
        if (-reproj_Zero > -reproj_One)
        {
            // the far plane is mapped to 0 -> inverse DX style
            mFarPlane = -reproj_Zero;
            mNearPlane = -reproj_One;
        }
        else
        {
            // the near plane is mapped to zero -> DX style
            mFarPlane = -reproj_One;
            mNearPlane = -reproj_Zero;
        }
    }
    else
    {
        // OpenGL style, mapping to -1..1 or 1..-1
        if (-reproj_minusOne > -reproj_One)
        {
            // the far plane is mapped to -1 -> inverse GL style
            mFarPlane = -reproj_minusOne;
            mNearPlane = -reproj_One;
        }
        else
        {
            // the near plane is mapped to -1 -> GL style
            mFarPlane = -reproj_One;
            mNearPlane = -reproj_minusOne;
        }
    }
}
