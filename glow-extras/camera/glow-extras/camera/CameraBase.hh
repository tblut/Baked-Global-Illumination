#pragma once

#include <glm/fwd.hpp>

#include <glow/common/shared.hh>

namespace glow
{
namespace camera
{
GLOW_SHARED(class, CameraBase);
/**
 * @brief Common interface for cameras
 *
 * This interface only contains getter on purpose.
 * All logic that wants to modify a camera should know the actual structure of the camera and therefore use the
 * specific subclass.
 */
class CameraBase
{
protected:
    CameraBase();

public:
    virtual ~CameraBase();

    /**
     * @brief gets the Position of the camera
     * @return a 3-dimensional position in the global coordinate system
     */
    virtual glm::vec3 getPosition() const = 0;
    /**
     * @brief gets the ViewMatrix of the camera
     * @return a 4x4 matrix containing projection independent camera transforms
     */
    virtual glm::mat4 getViewMatrix() const = 0;
    /**
     * @brief gets the ProjectionMatrix of the camera
     * @return a 4x4 matrix containing the projection into normalized device coordinates
     */
    virtual glm::mat4 getProjectionMatrix() const = 0;
    /**
     * @brief gets the ViewportSize of the current viewport of this camera
     * @return the 2-dimensional size of the viewport
     */
    virtual glm::uvec2 getViewportSize() const = 0;
    /**
     * @brief Gets the near clipping plane as a distance from the camera.
     * @return the near clipping plane
     */
    virtual float getNearClippingPlane() const = 0;
    /**
     * @brief Gets the far clipping plane as a distance from the camera. Note that it could be inf!
     * Not all projection matrices have a real far plane but some are unlimited!
     * @return the near clipping plane
     */
    virtual float getFarClippingPlane() const = 0;
};
}
}
