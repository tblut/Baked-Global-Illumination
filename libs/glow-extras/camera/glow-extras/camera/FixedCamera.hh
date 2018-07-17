#pragma once

#include "CameraBase.hh"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace glow
{
namespace camera
{
GLOW_SHARED(class, FixedCamera);
/**
 * @brief A fixed camera where all attributes are set explicitly except for the near/far plane
 *        which are derrived from the projection matrix.
 */
class FixedCamera : public CameraBase
{
private:
    glm::vec3 mPosition;
    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::uvec2 mViewportSize;

    // will get calculated based on the projection matrix
    // so there are no explicit setters for this!
    float mNearPlane;
    float mFarPlane;

public:
    /// CAUTION: default ctor with zero values
    FixedCamera();
    FixedCamera(const glm::vec3& _pos, const glm::mat4& _view, const glm::mat4& _proj, const glm::uvec2& _viewport);

    // Getter, Setter for Camera Position
    virtual glm::vec3 getPosition() const override { return mPosition; }
    virtual void setPosition(glm::vec3 const& _val) { mPosition = _val; }
    // Getter, Setter for Camera ViewMatrix
    virtual glm::mat4 getViewMatrix() const override { return mViewMatrix; }
    virtual void setViewMatrix(glm::mat4 const& _val) { mViewMatrix = _val; }
    // Getter, Setter for Camera ProjectionMatrix
    virtual glm::mat4 getProjectionMatrix() const override { return mProjectionMatrix; }
    virtual void setProjectionMatrix(glm::mat4 const& _val);

    // Getter, Setter for Camera ViewportSize
    virtual glm::uvec2 getViewportSize() const override { return mViewportSize; }
    virtual void setViewportSize(glm::uvec2 const& _val) { mViewportSize = _val; }
    // getters for near/far plane (far can be inf!)
    virtual float getNearClippingPlane() const override { return mNearPlane; }
    virtual float getFarClippingPlane() const override { return mFarPlane; }
};
}
}
